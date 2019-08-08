
// Not a pi
if [[ ! -f /sys/firmware/devicetree/base/model ]]; then 
	echo -1 
	exit
fi

echo $(cat /sys/firmware/devicetree/base/model | cut -d " " -f 3)