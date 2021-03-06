# CCAL - C calendar

Similar to calcurse but even simpler.

```
________/\\\\\\\\\________/\\\\\\\\\_____/\\\\\\\\\_____/\\\_____________        
 _____/\\\////////______/\\\////////____/\\\\\\\\\\\\\__\/\\\_____________       
  ___/\\\/_____________/\\\/____________/\\\/////////\\\_\/\\\_____________      
   __/\\\______________/\\\_____________\/\\\_______\/\\\_\/\\\_____________     
    _\/\\\_____________\/\\\_____________\/\\\\\\\\\\\\\\\_\/\\\_____________    
     _\//\\\____________\//\\\____________\/\\\/////////\\\_\/\\\_____________   
      __\///\\\___________\///\\\__________\/\\\_______\/\\\_\/\\\_____________  
       ____\////\\\\\\\\\____\////\\\\\\\\\_\/\\\_______\/\\\_\/\\\\\\\\\\\\\\\_ 
        _______\/////////________\/////////__\///________\///__\///////////////__
```


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

## Generating a schedule

The schedule is generated with the option -g, getting goals information from the CSV file goals.txt, located in ~/.config/ccal. The goals.txt file format is as follows:

`# Description,Duration(float)(hours),Expected days to event(float),Repeating(0/1),Lower Bound(float)(time),Upper Bound(float)(time)`

## Configuration

The date format can be defined in the DATE\_FMT macro.

## Dependencies

No dependencies other than the standard library.

## Building

With `make`
