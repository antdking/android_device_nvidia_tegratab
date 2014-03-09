#!/system/bin/sh

TARGET=/sys/class/ieee80211/phy0

# wait up to 5 seconds
x=1
while [ $x -le 50 ]; do
	if [ -d $TARGET ]; then
		break;
	fi
	x=$(( $x + 1 ))
	sleep 0.1
done

/system/bin/iw phy0 interface add p2p0 type managed

