PID=`ps -aef | grep -i matrix-server | grep -v grep | awk '{print $2}'`
if [[ ! -z "${PID}" ]]; then
	kill ${PID}
fi
