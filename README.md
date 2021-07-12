# CCAL - C calendar

Similar to calcurse but even simpler.

![Screenshot of ccal's `week` output](example.png)

```
Usage:
    -n --new - Add new event
    -p - Print all events
    -g --generate <date> - Generate schedule
    -d --delete <id> - Delete event with ID <id>
    -q --query <date> - Query events on <date>
    -w <date> - Print week's worth of events from <date>
    -q --query <date> -s <id> - Skip event with <id> on <date>
    -q --query <date> --clear-date - Skip all events on <date>
    -q --query <date> -f <num> - Print events for <num> of dates from <date>
```

## Configuration

Change the `DATE_FMT` and `DATA_DIR` as wanted

```
#define DATE_FMT "%d/%m/%Y"
#define DATA_DIR "/home/nikola/.config/ccal/events"
```

## Dependencies

No dependencies other than the standard library.
