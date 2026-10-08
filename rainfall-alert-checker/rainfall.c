/*
 * Rainfall Alert Checker
 * ----------------------
 * Reads the last 24 hours of rainfall for each district from a text file,
 * labels every reading with the India Meteorological Department (IMD)
 * rainfall category, and flags each district with heavy rain or more.
 * The report is printed on screen and also saved to alert_report.txt.
 *
 * IMD rainfall categories (rainfall in 24 hours):
 *   Very light        trace to 2.4 mm
 *   Light             2.5 to 15.5 mm
 *   Moderate          15.6 to 64.4 mm
 *   Heavy             64.5 to 115.5 mm
 *   Very heavy        115.6 to 204.4 mm
 *   Extremely heavy   204.5 mm or more
 * (0 mm is shown as "No rain".)
 *
 * Compile:  gcc rainfall.c -o rainfall
 * Run:      rainfall              (reads rainfall.txt)
 *           rainfall other.txt    (reads another file)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_DISTRICTS 50
#define MAX_NAME      40
#define MAX_LINE      200
#define DEFAULT_FILE  "rainfall.txt"
#define REPORT_FILE   "alert_report.txt"
#define NUM_CATEGORIES 7
#define FIRST_ALERT_CATEGORY 4   /* "Heavy" - this category and above get an ALERT */

struct Reading {
    char   district[MAX_NAME];
    double rainfall_mm;
};

const char *CATEGORY_NAMES[NUM_CATEGORIES] = {
    "No rain", "Very light", "Light", "Moderate",
    "Heavy", "Very heavy", "Extremely heavy"
};

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

/* Removes spaces and line endings (\n, or \r\n on Windows) from both ends. */
void trim(char *text)
{
    int start = 0;
    int end = (int)strlen(text) - 1;

    while (text[start] != '\0' && isspace((unsigned char)text[start]))
        start++;
    while (end >= start && isspace((unsigned char)text[end]))
        end--;

    memmove(text, text + start, end - start + 1);
    text[end - start + 1] = '\0';
}

/* Returns the IMD category number (an index into CATEGORY_NAMES). */
int category_of(double mm)
{
    if (mm <= 0.0)   return 0;   /* No rain         */
    if (mm < 2.5)    return 1;   /* Very light      */
    if (mm < 15.6)   return 2;   /* Light           */
    if (mm < 64.5)   return 3;   /* Moderate        */
    if (mm < 115.6)  return 4;   /* Heavy           */
    if (mm < 204.5)  return 5;   /* Very heavy      */
    return 6;                    /* Extremely heavy */
}

int is_alert(double mm)
{
    return category_of(mm) >= FIRST_ALERT_CATEGORY;
}

/* ------------------------------------------------------------------ */
/* Reading the data                                                    */
/* ------------------------------------------------------------------ */

/*
 * Turns one line like "Mandi,210.3" into a Reading.
 * Returns 1 if the line is valid. If not, writes the reason into `error`
 * and returns 0, so a bad line is reported instead of silently ignored.
 */
int parse_reading(char *line, struct Reading *r, char *error, int error_size)
{
    char *comma = strrchr(line, ',');
    char *district;
    char *value;
    char *end;
    double mm;

    if (comma == NULL) {
        snprintf(error, (size_t)error_size, "expected district,rainfall_mm");
        return 0;
    }
    *comma = '\0';          /* split the line into two strings */
    district = line;
    value = comma + 1;
    trim(district);
    trim(value);

    if (strlen(district) == 0) {
        snprintf(error, (size_t)error_size, "district name is empty");
        return 0;
    }
    if (strlen(district) >= MAX_NAME) {
        snprintf(error, (size_t)error_size, "district name is longer than %d letters", MAX_NAME - 1);
        return 0;
    }

    mm = strtod(value, &end);
    if (value[0] == '\0' || *end != '\0') {
        snprintf(error, (size_t)error_size, "rainfall must be a number in mm (got \"%s\")", value);
        return 0;
    }
    if (!(mm >= 0.0 && mm < 10000.0)) {
        snprintf(error, (size_t)error_size, "rainfall must be between 0 and 10000 mm");
        return 0;
    }

    strcpy(r->district, district);
    r->rainfall_mm = mm;
    return 1;
}

/* Reads every reading in the file. Returns how many lines were skipped. */
int load_readings(const char *filename, struct Reading list[], int *count)
{
    FILE *file = fopen(filename, "r");
    char line[MAX_LINE];
    char error[120];
    int line_number = 0;
    int skipped = 0;

    *count = 0;
    if (file == NULL) {
        printf("Could not open %s\n", filename);
        return 0;
    }

    while (fgets(line, sizeof line, file) != NULL) {
        struct Reading r;

        line_number++;
        trim(line);
        if (line[0] == '\0' || line[0] == '#')
            continue;   /* blank line or comment */

        if (*count >= MAX_DISTRICTS) {
            printf("Skipped line %d: too many districts (%d max)\n", line_number, MAX_DISTRICTS);
            skipped++;
        } else if (parse_reading(line, &r, error, sizeof error)) {
            list[*count] = r;
            (*count)++;
        } else {
            printf("Skipped line %d: %s\n", line_number, error);
            skipped++;
        }
    }
    fclose(file);
    return skipped;
}

/* ------------------------------------------------------------------ */
/* Sorting and the report                                              */
/* ------------------------------------------------------------------ */

/* Selection sort: wettest district first. */
void sort_by_rainfall(struct Reading list[], int count)
{
    for (int i = 0; i < count - 1; i++) {
        int wettest = i;

        for (int j = i + 1; j < count; j++) {
            if (list[j].rainfall_mm > list[wettest].rainfall_mm)
                wettest = j;
        }
        if (wettest != i) {
            struct Reading temp = list[i];
            list[i] = list[wettest];
            list[wettest] = temp;
        }
    }
}

/*
 * Prints the report to `out`. Passing stdout prints it on screen;
 * passing an open file writes the same report into that file.
 */
void print_report(FILE *out, const struct Reading list[], int count, int skipped)
{
    int per_category[NUM_CATEGORIES] = {0};
    int alerts = 0;
    double total = 0.0;

    fprintf(out, "RAINFALL REPORT - last 24 hours, wettest first\n");
    fprintf(out, "%-18s %14s   %-16s %s\n", "District", "Rainfall (mm)", "IMD category", "Alert");
    fprintf(out, "--------------------------------------------------------------\n");

    for (int i = 0; i < count; i++) {
        int category = category_of(list[i].rainfall_mm);

        fprintf(out, "%-18s %14.1f   %-16s %s\n",
                list[i].district, list[i].rainfall_mm,
                CATEGORY_NAMES[category],
                is_alert(list[i].rainfall_mm) ? "ALERT" : "");
        per_category[category]++;
        total += list[i].rainfall_mm;
        if (is_alert(list[i].rainfall_mm))
            alerts++;
    }

    fprintf(out, "--------------------------------------------------------------\n");
    fprintf(out, "Districts checked : %d\n", count);
    fprintf(out, "Average rainfall  : %.1f mm\n", total / count);
    fprintf(out, "Wettest district  : %s, %.1f mm\n", list[0].district, list[0].rainfall_mm);
    fprintf(out, "On alert          : %d (heavy rain or more)\n", alerts);

    /* Most severe category first; a comma goes before every item except the first. */
    int printed = 0;
    fprintf(out, "By category       : ");
    for (int c = NUM_CATEGORIES - 1; c >= 0; c--) {
        if (per_category[c] > 0) {
            fprintf(out, "%s%s %d", printed > 0 ? ", " : "", CATEGORY_NAMES[c], per_category[c]);
            printed++;
        }
    }
    fprintf(out, "\n");

    if (skipped > 0)
        fprintf(out, "Lines skipped     : %d (bad data - listed on screen when the file was read)\n", skipped);

    fprintf(out, "\nCategories follow IMD's 24-hour rainfall classification.\n");
    fprintf(out, "This is a learning project, not an official weather warning.\n");
}

int main(int argc, char *argv[])
{
    struct Reading list[MAX_DISTRICTS];
    int count = 0;
    const char *filename = (argc > 1) ? argv[1] : DEFAULT_FILE;
    int skipped;
    FILE *report;

    printf("=== Rainfall Alert Checker ===\n");
    skipped = load_readings(filename, list, &count);
    if (count == 0) {
        printf("No valid readings found in %s - nothing to report.\n", filename);
        return 1;
    }
    printf("Loaded %d reading%s from %s\n\n", count, count == 1 ? "" : "s", filename);

    sort_by_rainfall(list, count);
    print_report(stdout, list, count, skipped);

    report = fopen(REPORT_FILE, "w");
    if (report == NULL) {
        printf("\nCould not save the report to %s\n", REPORT_FILE);
        return 1;
    }
    print_report(report, list, count, skipped);
    fclose(report);
    printf("\nReport saved to %s\n", REPORT_FILE);
    return 0;
}
