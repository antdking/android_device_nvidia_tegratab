/*
#
# Copyright (c) 2012-2013, NVIDIA CORPORATION.  All rights reserved.
#
# NVIDIA CORPORATION and its licensors retain all intellectual property
# and proprietary rights in and to this software, related documentation
# and any modifications thereto.  Any use, reproduction, disclosure or
# distribution of this software and related documentation without an express
# license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#define LOG_TAG "volt_cap"
#include <errno.h>
#include <stdlib.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <linux/netlink.h>
#include "cpu_volt_cap.h"

#define VC_DEBUG 1
#if VC_DEBUG
#define log(...) ALOGD("VC: "__VA_ARGS__)
#else
#define log(...) {};
#endif


#define LOG_ALL         -1
#define LOG_EVENTS      1
#define LOG_FILE "/data/vc_log.txt"
#define DATA_FILE "/data/device_config.txt"
#define SAFE_USER_POINT_LIMIT 100
#define VOLT_FILE "/sys/kernel/tegra_cpu_volt_cap/volt"
#define CAPPING_ENABLE_FILE "/sys/kernel/tegra_cpu_volt_cap/capping_state"
#define STATS_FILE "/sys/power/tegra_rail_stats"
#define PATH    "/sys/class/thermal"
#define FILETYPE "CPU-therm"

#define THREAD_DATA_INIT(function, data)\
{ \
    .f = function,\
    .d = data, \
}

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t save_mutex = PTHREAD_MUTEX_INITIALIZER;
static int debug_params;
static char thermal_filepath[100];
static char thermal_zonepath[100];
static struct saved_data sd;
static float *btable;
static int btable_size;
static int *temperatures;
static int temp_list_size;

struct {
    struct rail_stats *saved_rs;
    int volt_steps;
    int temp_steps;
    int cur_temp_index;
    float session_points;
} vc;

static void log_events(const char *fmt, ...)
{
    FILE *fp = fopen(LOG_FILE, "a");
    va_list ap;
    va_start(ap, fmt);
    if ((debug_params & LOG_EVENTS) && fp) {
        struct tm *local;
        time_t t;
        t = time(NULL);
        local = localtime(&t);
        fprintf(fp, "%s ", asctime(local));
        vfprintf(fp, fmt, ap);
    }
    if (fp)
        fclose(fp);

    va_end(ap);
}

static void sysfs_write(const char *path, const char *s)
{
    char buf[80];
    int len;
    int fd = open(path, O_RDWR);

    if (fd < 0) {
        strerror_r(errno, buf, sizeof(buf));
        log("Error opening %s: %s\n", path, buf);
        return;
    }

    len = write(fd, s, strlen(s));
    if (len < 0) {
        strerror_r(errno, buf, sizeof(buf));
        log("Error writing to %s: %s\n", path, buf);
    }
    close(fd);
}

static int diff_time(struct timespec start,
                struct timespec finish,
                struct timespec *diff)
{
    if ((finish.tv_nsec - start.tv_nsec) < 0) {
        diff->tv_sec = finish.tv_sec-start.tv_sec-1;
        diff->tv_nsec = 1000000000+finish.tv_nsec-start.tv_nsec;
    } else {
        diff->tv_sec = finish.tv_sec-start.tv_sec;
        diff->tv_nsec = finish.tv_nsec-start.tv_nsec;
    }

    return 0;
}

static int add_nsec(struct timespec *ts, unsigned long ns)
{
    unsigned long temp;

    ts->tv_nsec += ns;
    if (ts->tv_nsec >= 1000000000) {
        ts->tv_nsec -= 1000000000;
        ts->tv_sec++;
    }

    return 0;
}

static int add_time(struct timespec *dst, const struct timespec *src)
{
    dst->tv_sec += src->tv_sec;
    add_nsec(dst, src->tv_nsec);

    return 0;
}

static unsigned int get_crcb(char *cp, unsigned int size)
{
    unsigned int crc;
    unsigned int mask;
    int j;

    crc = 0xFFFFFFFF;

    while (size--) {
        crc = crc ^ *cp++;
        for (j = 7; j >= 0; j--) {
            mask = (crc & 1) ? 0xEDB88320 : 0;
            crc = (crc >> 1) ^ mask;
        }
    }
    return ~crc;
}

static int get_xindex(int temp)
{
    int i = 0;
    int temps = temp_list_size;

    while (i < temps) {
        if (temperatures[i] >= temp)
            return i;
        i++;
    }

    return temp > temperatures[i-1] ? temps - 1 : -1;
}

static int get_yindex(int volt)
{
    int i = btable_size;
    int temps = temp_list_size;
    int j = 0;
    int count = 0;

    while (j < i) {
        if ((float)volt == btable[j])
            return count;
        count++;
        j = (temps + 1) * count;
    }

    return -1;
}

/*
 * save_to_file :
 * Saves the available points of user
 * Creates a crc value for the structure.
 * Adds the time of file updation.
 */
static void save_to_file(float points_to_save)
{
    int len;
    unsigned int crc;
    int fd;

    clock_gettime(CLOCK_REALTIME, &sd.saved_time);
    sd.balance_points = points_to_save;
    crc = get_crcb((void *)&sd, sizeof(sd));
    fd = open(DATA_FILE, O_RDWR);
    len = write(fd, &sd, sizeof(sd));
    len = write(fd, &crc, sizeof(unsigned int));
    close(fd);
}

/* Events supported by Thermal Netlink */
enum events {
    THERMAL_AUX0,
    THERMAL_AUX1,
    THERMAL_CRITICAL,
    THERMAL_DEV_FAULT,
};

struct thermal_genl_event {
    int orig;
    enum events event;
};

static int read_temp_update(int sock_fd, struct temperatures_stats *ts)
{
    char buf[100];
    int result;
    struct sockaddr_nl dest_addr;
    struct iovec iov;
    struct msghdr msg;
    struct thermal_genl_event *pgenl;

    iov.iov_base = buf;
    iov.iov_len = 100;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    result = recvmsg(sock_fd, &msg, 0);
    if (result == -1) {
        log("recvmsg failed, error is %d\n", result);
        return -1;
    }
    pgenl = NLMSG_DATA(buf);
    /* skip genl header */
    pgenl += 1;
    log("Trip=%d, Event=%d\n", pgenl->orig, pgenl->event);
    ts->current_temp = pgenl->orig;
    ts->is_rise = pgenl->event;

    return 0;
}

static int read_cpu_rail_sysfs(struct rail_stats *rs, int *rail_num)
{
    FILE *fp = fopen(STATS_FILE,"r");
    float mV;
    char buff[100];

    *rail_num = 0;
    if (fp) {
        fscanf(fp, "millivolts   time\n");
        fscanf(fp, "vdd_cpu (bin: %fmV)\n", &mV);
        while (1) {
            fscanf(fp, "%s", buff);
            if (!strncmp(buff, "vdd_core", strlen("vdd_core"))) {
                break;
            }
            rs->milli_volts = strtol(buff, NULL, 10);
            fscanf(fp, "%s\n", buff);
            rs->time_10msec = strtol(buff, NULL, 10);
            rs++;
            ++(*rail_num);
        }
    } else
        log("fopen %s failed", STATS_FILE);

    fclose(fp);
    return 0;
}

/*
 * update_state - Function to gather temp and voltage statistics.
 * On any of the events thread takes following action.
 * 1.) Reads CPU temperature through sysfs.
 * 2.) Reads the CPU rail voltage and time stats through sysfs.
 * 3.) Subtracts new volt vs time stats with the saved volt vs time
 * stats (Present at temperature = 0).
 * 4.) The value of 3.) is places in temperature basket of previously
 * read temperature.
 * 5.) Old temperature is updated to reflect current temperature.
 * 6.) The complete table of temperature vs volt is multiplied with
 * table provided at
 * https://wiki.nvidia.com/wmpwiki/index.php/Tegra_T35/Power/Hyper-Voltaging
 * The conversion is done for ms to hour burnout.
 * 7.) The resultant is a total_burnout for this session.
 * 8.) If points available is greater than a safe value (100 points)
 * then nothing is done.
 * 9.) If points are less than 100, a capping voltage currently 1000mv
 * is set and voltage capping is enabled.
 * 10.) The table is written to log file on thread trigger.
 *
 */

static int update_state(struct rail_stats *rs,
            int rs_num,
            struct temperatures_stats *ts,
            float add_points,
            float *balance_points)
{
    pthread_mutex_lock(&mutex);
    // Delete burned points
    if (rs && ts) {
        int i = 0;
        int vrow = 0;
        int long time_diff = 0;
        int tcol = vc.cur_temp_index;

        while (i != rs_num) {
            vrow = get_yindex(rs->milli_volts);
            if (vrow >= 0) {
                time_diff = rs->time_10msec - vc.saved_rs[vrow].time_10msec;
                vc.saved_rs[vrow].time_10msec = rs->time_10msec;
                vc.session_points -= (btable[vrow * (vc.temp_steps+1) + tcol + 1] * (float)time_diff)/100;
            }

            rs++;
            i++;
        }

        i = get_xindex(ts->current_temp);
        if (!ts->is_rise)
            i = (i == 0 ? 0 : i - 1);

        if (i >= 0)
            vc.cur_temp_index = i;

        log("Thermal update change is %f\n", vc.session_points);
    } else if (rs) {
        int i = 0;
        int vrow = 0;
        int long time_diff;
        int tcol = vc.cur_temp_index;

        while (i != rs_num) {
            vrow = get_yindex(rs->milli_volts);
            if (vrow >= 0) {
                time_diff = rs->time_10msec - vc.saved_rs[vrow].time_10msec;
                vc.saved_rs[vrow].time_10msec = rs->time_10msec;
                vc.session_points -= (btable[vrow * (vc.temp_steps+1) + tcol + 1] * (float)time_diff)/100;
            }
            rs++;
            i++;
        }

        log("Volt update change is %f\n", vc.session_points);
    } else if (add_points) {
        log("Points added %f\n", add_points);
        vc.session_points += add_points;
    }

    if (balance_points) {
        *balance_points = vc.session_points;
        log("Balance points in session %f\n", vc.session_points);
    }

    if (vc.session_points <= SAFE_USER_POINT_LIMIT) {
        sysfs_write(VOLT_FILE, "1300");
        sysfs_write(CAPPING_ENABLE_FILE, "1");
    } else
        sysfs_write(CAPPING_ENABLE_FILE, "0");

    pthread_mutex_unlock(&mutex);
    return 0;
}

static void *temp_record(void *x)
{
    struct temperatures_stats ts;
    struct rail_stats *rail_readings;
    int rail_num;
    int result;
    int sock_fd;
    struct sockaddr_nl src_addr;

    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (sock_fd == -1) {
        log("Socket failed!\n");
        return 0;
    }

    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    src_addr.nl_groups = 2;
    result = bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
    if (result) {
        log("Bind failed! %d.\n", result);
        return 0;
    }

    rail_readings = (struct rail_stats*)malloc(sizeof(struct rail_stats) * vc.volt_steps);
    /* Loop */
    while (1) {
        if (read_temp_update(sock_fd, &ts) < 0)
            continue;
        read_cpu_rail_sysfs(rail_readings, &rail_num);
        update_state(rail_readings, rail_num, &ts, -1, NULL);
    }

    free(rail_readings);
    return 0;
}

/*
 * volt_record - Thread create event to recalculate burnout.
 * Thread execution - Thread sleeps for voltage polling interval
 * currently 5 minutes. When it is woken up it signals the
 * stats update thread.
 */
#define VOLT_POLL_SECONDS (5*60)
static void *volt_record(void *x)
{
    struct rail_stats *rail_readings;
    int rail_num;

    rail_readings = (struct rail_stats*)malloc(sizeof(struct rail_stats) * vc.volt_steps);
    /* Loop */
    while (1) {
        sleep(VOLT_POLL_SECONDS);
        read_cpu_rail_sysfs(rail_readings, &rail_num);
        update_state(rail_readings, rail_num, NULL, -1, NULL);
    }

    free(rail_readings);
    return 0;
}


/*
 * points_update_thread - Thread to update the available points. This
 * also saves the points to a device_config.txt file for availability
 * across boots.
 *
 * Thread execution -
 * 1.) Thread sleeps for the points update interval
 * 2.) Thread saves the time before sleep and reads time after resume.
 * 3.) The timer used is the CLOCK_MONOTONIC type of clock this gives
 * the time from a predefined reference. Thus changes to system clock
 * have no effect on the calculations.
 * 4.) The sleep time is used to calculate the amount of points to be
 * credited to user.
 *
 */
#define UPDATE_EVERY_HRS    (1)
#define SAVE_EVERY_HRS  (6)
#define SECS_IN_HOUR    (60*60)
#define DAILY_POINTS_UPDATE (100.0)
int save_secs = SAVE_EVERY_HRS * 60 * 60;
unsigned long sleep_secs;
static void *points_record(void *data)
{
    struct timespec start, finish, diff, save;
    int save_secs = SAVE_EVERY_HRS * 60 * 60;
    long int saved_ns = 0;
    float points_per_sec = 0.0;
    float points_to_add = 0.0;
    float points_to_save = 0.0;

    sleep_secs = (UPDATE_EVERY_HRS > SAVE_EVERY_HRS) ? SAVE_EVERY_HRS : UPDATE_EVERY_HRS;
    sleep_secs *= SECS_IN_HOUR;
    points_per_sec = DAILY_POINTS_UPDATE/((float)(24*60*60));
    memset(&save, 0, sizeof(struct timespec));

    while(1) {
        /* Point add and save */
        clock_gettime(CLOCK_MONOTONIC, &start);
        sleep(sleep_secs);
        clock_gettime(CLOCK_MONOTONIC, &finish);

        diff_time(start, finish, &diff);
        add_nsec(&diff, saved_ns);
        saved_ns = diff.tv_nsec;
        points_to_add = diff.tv_sec * points_per_sec;
        update_state(NULL, 0, NULL, points_to_add, &points_to_save);

        add_time(&save, &diff);
        if (save.tv_sec >= save_secs) {
            save.tv_sec = 0;
            save.tv_nsec = 0;
            pthread_mutex_lock(&save_mutex);
            save_to_file(points_to_save);
            pthread_mutex_unlock(&save_mutex);
            log("Saved to file \n");
        }
    }

    return 0;
}


struct thread_data threads[] = {
    THREAD_DATA_INIT(points_record, NULL),
    THREAD_DATA_INIT(temp_record, NULL),
    THREAD_DATA_INIT(volt_record, NULL),
};

static void cleanup_function(int x)
{
    int sig;
    int err;
    sigset_t set;
    unsigned i;

    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGINT);
    sigprocmask(SIG_BLOCK, &set, NULL);
    err = sigwait(&set, &sig);
    sigprocmask(SIG_UNBLOCK, &set, NULL);
    log_events("Program exiting\n");
    log("cleanup function received signal %d err %d\n", sig, err);

    pthread_mutex_lock(&save_mutex);
    save_to_file(vc.session_points);
    pthread_mutex_unlock(&save_mutex);

}

/*
 * init_function : Finds the saved context file.
 * Validates sanity by crc check.
 * If sanity check pass then adds points equal to the interval
 * difference between current time and saved time and
 * adds the points to structure.
 * If sanity check fails currently provides user a 2100 points credit,
 * need to recreate this based on aging and other factors.
 */

static int init_function(void)
{
    int fd;
    unsigned len;
    int valid_file = 0;
    struct saved_data sd;
    unsigned read_crc, calc_crc;
    struct timespec current_time, diff;
    unsigned int total_intervals;

    get_burnout_table(&btable, &btable_size);
    get_temperature_list(&temperatures, &vc.temp_steps);
    temp_list_size = vc.temp_steps;
    vc.volt_steps = btable_size / (vc.temp_steps + 1);
    vc.saved_rs = (struct rail_stats *)malloc(sizeof(struct rail_stats) * vc.volt_steps);
    fd = open(DATA_FILE, O_RDWR);
    if (fd <= 0) {
        fd = open(DATA_FILE, O_CREAT | O_RDWR, 0666);
    } else {
        len = read(fd, &sd, sizeof(sd));
        if (len < sizeof(sd))
            goto exit;
        calc_crc = get_crcb((void *)&sd, sizeof(sd));
        len = read(fd, &read_crc, sizeof(unsigned int));
        if (len <= 0)
            goto exit;
        if (read_crc == calc_crc)
            valid_file++;
    }

exit:
    close(fd);
    if (valid_file) {
        log("File validated\n");
        clock_gettime(CLOCK_REALTIME, &current_time);
        if (current_time.tv_sec <= sd.saved_time.tv_sec) {
            /* OS image update has reset time */
            /* Read the points saved previously */
            vc.session_points = sd.balance_points;
            return 0;
        }

        diff_time(current_time, sd.saved_time, &diff);
        vc.session_points = diff.tv_sec * (DAILY_POINTS_UPDATE/((float)(24*60*60)));
        vc.session_points += sd.balance_points;
    } else {
        /* Find the device age but provide boost points as of now */
        /* Add a 21 day boost points */
        vc.session_points = 2100;
    }
    return 0;
}

static int init_thermal_path(void)
{
    int file = 0;
    FILE *fp;
    char path[100];
    char buf[100];
    int err = -1;

    while (file != 10) {
        snprintf(path, sizeof(path), "%s/thermal_zone%d/type", PATH, file);
        fp = fopen(path, "r");
        if (!fp)
            break;
        fgets(buf, sizeof(buf), fp);
        if (!strncmp(buf, FILETYPE, strlen(FILETYPE))) {
            snprintf(thermal_filepath, sizeof(thermal_filepath),
                "%s/thermal_zone%d/temp", PATH, file);
            snprintf(thermal_zonepath, sizeof(thermal_zonepath),
                "%s/thermal_zone%d", PATH, file);
            err = 0;
            break;
        }

        fclose(fp);
        file++;
    }

    log("TEMP PATH:%s\n", thermal_filepath);
    return err;
}

static void print_usage(void)
{
    log("usage: voltcapd [options]\n");
    log("-a:log all debug data\n");
    log("-e:log all event data\n");
}

static int read_data_cmd(FILE *fp)
{
    int i;

    log("cur_temp_index: %d\n", vc.cur_temp_index);
    i = vc.volt_steps;
    while (i--) {
        log("saved_rs: %dmV %ld 10msec\n", vc.saved_rs[i].milli_volts, vc.saved_rs[i].time_10msec);
    }
    log("session_points: %f\n", vc.session_points);
    log("temp_steps %d\n", vc.temp_steps);
    log("volt_steps %d\n", vc.volt_steps);
    log("Thermal path: %s\n", thermal_filepath);
    log("Thermal zone: %s\n", thermal_zonepath);

    return 0;
}

static int read_points_cmd(FILE *fp)
{
    log("balance_points: %f\n", sd.balance_points);
    log("saved_time:secs %ld nsecs %ld", (long)sd.saved_time.tv_sec, sd.saved_time.tv_nsec);

    return 0;
}

static int write_points_cmd(FILE *fp)
{
    float points;

    fscanf(fp, "%f\n", &sd.balance_points);
    log("Points added:%f\n", sd.balance_points);
    vc.session_points = sd.balance_points;
    return 0;
}

static int write_sleep_time_cmd(FILE *fp)
{
    int val;

    fscanf(fp, "%lu", &sleep_secs);
    log("Sleep interval %lu s", sleep_secs);
    return 0;
}

static int write_save_time_cmd(FILE *fp)
{
    int val;

    fscanf(fp, "%d", &save_secs);
    log("Point save interval %d s", save_secs);
    return 0;
}

struct command commands[] = {
            {"read_data", read_data_cmd},
            {"read_points", read_points_cmd},
            {"write_points", write_points_cmd},
            {"write_sleep_time", write_sleep_time_cmd},
            {"write_save_time", write_save_time_cmd},
            };

static int execute_command(char *cmd_buffer, FILE *fp)
{
    unsigned i;
    int ret_val;

    for (i = 0; i < sizeof(commands)/sizeof(struct command);i++)
        if (strncmp(cmd_buffer,
            commands[i].cmd, strlen(commands[i].cmd)) == 0) {
            ret_val = commands[i].f(fp);
            break;
        }

    return ret_val;
}

#define DIR "/data/dev_cmd"
static void *debug_cmd_thread(void *data)
{
    char cmd_buffer[50];
    unsigned i;
    int fd;
    FILE *fp;

    fd = mkfifo(DIR, 0666);
    if (fd == 0) {
        log("fifo created\n");
    } else
        return 0;

    while (1) {
        fp = fopen(DIR, "r");
        fgets(cmd_buffer, sizeof(cmd_buffer), fp);
        i = execute_command(cmd_buffer, fp);
        log("Read command:%s\n", cmd_buffer);
        fflush(fp);
        fclose(fp);
    }


    return 0;
}

int main(int argc, char *argv[])
{
    int err;
    unsigned int i;
    void *dev = NULL;
    int fd;

    /* Skip program name */
    argv++;
    argc--;
    while (argc != 0) {
        if (strcmp(argv[0], "-a") == 0)
            debug_params = LOG_ALL;
        else if (strcmp(argv[0], "-e") == 0)
            debug_params |= LOG_EVENTS;
        else
            print_usage();
        argc--;
        argv++;
    }

    /* command interface */
    if (debug_params) {
        struct thread_data dbg = THREAD_DATA_INIT(debug_cmd_thread ,NULL);
        pthread_create(&dbg.t ,NULL ,dbg.f ,dbg.d);
    }

    if (debug_params) {
        fd = open(LOG_FILE, O_CREAT | O_RDWR, 0666);
        if (fd > 0)
            close(fd);
    }

    init_function();
    if (init_thermal_path())
        return -1;

    for (i = 0; i < (sizeof(threads)/sizeof(struct thread_data)); i++) {
        pthread_create(&(threads[i].t), NULL,
                threads[i].f , threads[i].d);
    }

    cleanup_function(0);
    exit(0);
    return 0;
}
