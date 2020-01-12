## Accessing the files

Sample commands

- curl "192.168.1.80/fs"
- curl "192.168.1.80/fs/TESTFILE.INO"
- curl -X DELETE "192.168.1.80/fs/TESTFILE.INO"
- curl -H "Expect:" --upload-file 1.cfg "192.168.1.80/fs/"
- curl "192.168.1.80/fs/1.cfg" -o 1.cfg\_back

## The event logs

Event logs are ASCII files that start with the 'E' prefix and then a number that represents the number of hours since Jan 1st 1970. Inside an event log each line represents an event. Every line starts with a hexadecimal time-stamp following the UNIX epoch convention

The master controller will create a new file every hour and at all times will maintain 7 days worth of events

