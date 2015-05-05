#!/bin/sh
### BEGIN INIT INFO
# Provides:          LiberaBrilliancePlus
# Required-Start:    $local_fs $network $named $time $syslog
# Required-Stop:     $local_fs $network $named $time $syslog
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Description:       Tango Device Server for Libera Brilliance Plus
### END INIT INFO

SCRIPT=/usr/local/bin/LiberaBrilliancePlus/LiberaBrilliancePlus
RUNAS=root

PIDFILE=/var/run/LiberaBrilliancePlus.pid
LOGFILE=/var/log/LiberaBrilliancePlus.log

export TANGO_HOST="TANGO_HOST_PLACEHOLDER"
COLUMN=":"

start() {
  if case ${TANGO_HOST} in *"${COLUMN}"*) false;; *) true;; esac; then
    echo "Run the following script to configure the startup script:" >&2
    echo "    sudo /usr/local/bin/LiberaBrilliancePlus/configure.sh TANGOHOST TANGOINSTANCE" >&2
    echo "e.g.:" >&2
    echo "    sudo /usr/local/bin/LiberaBrilliancePlus/configure.sh 10.0.0.1:10000 1" >&2
    echo "Then start the service with:" >&2
    echo "    service \"LiberaBrilliancePlus\" start" >&2

    return 0
  fi

  if [ -f "$PIDFILE" ] && kill -0 $(cat "$PIDFILE"); then
    echo 'Service already running' >&2
    return 1
  fi
  echo 'Starting service…' >&2
  cd /usr/local/bin/LiberaBrilliancePlus
  local CMD="$SCRIPT TANGO_INSTANCE_PLACEHOLDER &> \"$LOGFILE\" & echo \$!"
  su -c "$CMD" $RUNAS > "$PIDFILE"
  echo 'Service started' >&2
}

stop() {
  if [ ! -f "$PIDFILE" ] || ! kill -0 $(cat "$PIDFILE"); then
    echo 'Service not running' >&2
    return 1
  fi
  echo 'Stopping service…' >&2
  kill -15 $(cat "$PIDFILE") && rm -f "$PIDFILE"
  echo 'Service stopped' >&2
}

uninstall() {
  echo -n "Are you really sure you want to uninstall this service? That cannot be undone. [yes|No] "
  local SURE
  read SURE
  if [ "$SURE" = "yes" ]; then
    stop
    rm -f "$PIDFILE"
    echo "Notice: log file is not be removed: '$LOGFILE'" >&2
    update-rc.d -f LiberaBrilliancePlus remove
    rm -fv "$0"
  fi
}

case "$1" in
  start)
    start
    ;;
  stop)
    stop
    ;;
  uninstall)
    uninstall
    ;;
  restart)
    stop
    start
    ;;
  *)
    echo "Usage: $0 {start|stop|restart|uninstall}"
esac
