#
# Regular cron jobs for the qtden package
#
0 4	* * *	root	[ -x /usr/bin/qtden_maintenance ] && /usr/bin/qtden_maintenance
