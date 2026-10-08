# Rainfall Alert Checker

A command-line program in C that reads 24-hour rainfall for each district, labels every reading with the official **India Meteorological Department (IMD)** rainfall category, and flags every district with heavy rain or more. The report is shown on screen and saved to a file.

## Why

Heavy rain is what sets off landslides and flash floods in Himachal Pradesh during the monsoon. A quick report that answers "which districts got heavy rain in the last 24 hours?" is the first thing a response team looks at. I built this while my team was exploring disaster-response ideas for **Hack on Hills 8.0**.

## IMD rainfall categories used

| Category | Rainfall in 24 hours | Alert? |
|---|---|---|
| Very light | trace to 2.4 mm | |
| Light | 2.5 – 15.5 mm | |
| Moderate | 15.6 – 64.4 mm | |
| Heavy | 64.5 – 115.5 mm | ALERT |
| Very heavy | 115.6 – 204.4 mm | ALERT |
| Extremely heavy | 204.5 mm or more | ALERT |

0 mm is shown as "No rain". Category ranges are taken from IMD bulletins ([IMD Hyderabad forecast bulletin](https://mausam.imd.gov.in/hyderabad/mcdata/tsfcst.pdf), [IMD RSMC New Delhi heavy rainfall bulletin](https://rsmcnewdelhi.imd.gov.in/uploads/archive/8/8_f9822f_heavy%20rainfall%201506.pdf)).

## Features

- Reads district rainfall from a text file (`rainfall.txt` by default)
- Labels each district with its IMD category and flags heavy rain or more
- Sorts districts from wettest to driest
- Summary: average rainfall, wettest district, number on alert, count per category
- Saves the same report to `alert_report.txt`
- **Bad lines are never silently ignored.** Each one is reported with its line number and the reason

## How to run

```
gcc rainfall.c -o rainfall
rainfall               (Windows)
./rainfall             (Linux / macOS)
```

To use a different data file: `rainfall myfile.txt`

## Input format

One district per line. Lines starting with `#` are comments.

```
district,rainfall_mm
Mandi,210.3
```

## Sample run

```
=== Rainfall Alert Checker ===
Skipped line 16: rainfall must be a number in mm (got "abc")
Loaded 11 readings from rainfall.txt

RAINFALL REPORT - last 24 hours, wettest first
District            Rainfall (mm)   IMD category     Alert
--------------------------------------------------------------
Mandi                       210.3   Extremely heavy  ALERT
Kangra                      132.5   Very heavy       ALERT
Kullu                        98.6   Heavy            ALERT
Hamirpur                     71.2   Heavy            ALERT
Sirmaur                      64.5   Heavy            ALERT
Shimla                       58.9   Moderate
Chamba                       48.0   Moderate
Solan                        15.6   Moderate
Bilaspur                     12.4   Light
Lahaul and Spiti              1.2   Very light
Kinnaur                       0.0   No rain
--------------------------------------------------------------
Districts checked : 11
Average rainfall  : 64.8 mm
Wettest district  : Mandi, 210.3 mm
On alert          : 5 (heavy rain or more)
By category       : Extremely heavy 1, Very heavy 1, Heavy 3, Moderate 3, Light 1, Very light 1, No rain 1
Lines skipped     : 1 (bad data - listed on screen when the file was read)

Categories follow IMD's 24-hour rainfall classification.
This is a learning project, not an official weather warning.

Report saved to alert_report.txt
```

The `Una,abc` line in the sample file is wrong on purpose, to show how bad data is reported.

## C concepts used

Structs, arrays, file reading and writing, passing a `FILE *` so one function can print to the screen or to a file, string handling (`strrchr`, `strtod`), input validation, selection sort, and `#define` constants.

## Limitations

- **The rainfall numbers in `rainfall.txt` are sample data, not real IMD readings.**
- This is a learning project. It does not issue official warnings. For real warnings, follow IMD and the State Disaster Management Authority.
- Rainfall category alone does not predict landslides. Slope, soil and earlier rain matter too.
- Holds up to 50 districts.

## Ideas for next steps

- Read several days of data and show the total over 3 days
- Show which districts got wetter or drier than the day before
- Let the user choose which category starts the alert
