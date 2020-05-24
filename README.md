# CCAL - C calendar

Kind of like calcurse, but even simpler. Used only for simple event tracking. No fancy features (yet ;)).

![Screenshot of ccal's `week` output](example.png)

```
Usage:
    -n --new - Add new event
    -p - Print all events
    -d --delete <id> - Delete event with ID <id>
    -q --query <date> - Query events on <date>
    -w <date> - Print week's worth of events from <date>
    -q --query <date> -s <id> - Skip event with <id> on <date>
```

## Configuration

Change the `DATE_FMT` and `DATA_DIR` as wanted

```
#define DATE_FMT "%d/%m/%Y"
#define DATA_DIR "/home/nikola/.config/ccal/events"
```

## Build

Just `make`. No dependencies other than glibc.
