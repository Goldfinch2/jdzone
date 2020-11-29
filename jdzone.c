#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <asm/types.h>
#include <fcntl.h>
#include <dirent.h>

#include <linux/input.h>

/* this macro is used to tell if "bit" is set in "array"
 * it selects a byte from the array, and does a boolean AND 
 * operation with a byte that only has the relevant bit set. 
 * eg. to check for the 12th bit, we do (array[1] & 1<<4)
 */
#define test_bit(bit, array)    (array[bit/8] & (1<<(bit%8)))

static int verbose_flag;

void print_axis_index(int index)
{
    switch (index)  {
        case ABS_X :
            printf(" X:        ");
            break;

        case ABS_Y :
            printf(" Y:        ");
            break;

        case ABS_Z :
            printf(" Z:        ");
            break;

        case ABS_RX :
            printf(" XRATE:    ");
            break;

        case ABS_RY :
            printf(" YRATE:    ");
            break;

        case ABS_RZ :
            printf(" ZRATE:    ");
            break;

        case ABS_THROTTLE :
            printf(" THROTTLE: ");
            break;

        case ABS_RUDDER :
            printf(" RUDDER:   ");
            break;

        case ABS_WHEEL :
            printf(" WHEEL:    ");
            break;

        case ABS_GAS :
            printf(" ACCEL:    ");
            break;

        case ABS_BRAKE :
            printf(" BRAKE:    ");
            break;

        case ABS_HAT0X :
            printf(" H0X:      ");
            break;

        case ABS_HAT0Y :
            printf(" H0Y:      ");
            break;

        case ABS_HAT1X :
            printf(" H1X:      ");
            break;

        case ABS_HAT1Y :
            printf(" H1Y:      ");
            break;

        case ABS_HAT2X :
            printf(" H2X:      ");
            break;

        case ABS_HAT2Y :
            printf(" H2Y:      ");
            break;

        case ABS_HAT3X :
            printf(" H3X:      ");
            break;

        case ABS_HAT3Y :
            printf(" H3Y:      ");
            break;

        case ABS_PRESSURE :
            printf(" PRESSURE: ");
            break;

        case ABS_DISTANCE :
            printf(" DISTANCE: ");
            break;

        case ABS_TILT_X :
            printf(" TILTX:    ");
            break;

        case ABS_TILT_Y :
            printf(" TILTY:    ");
            break;

        case ABS_MISC :
            printf(" MISC:     ");
            break;

        default:
            printf(" UNK:      ");
            break;
    }
}


int show_calibration(char *evdev)
{
    int     fd = -1;
    uint8_t abs_bitmask[ABS_MAX/8 + 1];
    int     yalv;
    float   deadzone_percent;
    struct  input_absinfo abs_features;
    int     ret = 1;

    if ((fd = open(evdev, O_RDONLY)) < 0) {
        perror("    Open");
        goto END;
    }

    memset(abs_bitmask, 0, sizeof(abs_bitmask));
    if (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask) < 0) {
        // perror("Get features mask");
        goto END;
    }

    for (yalv = 0; yalv < ABS_MAX; yalv++) {
        if (test_bit(yalv, abs_bitmask)) {
            printf("    %2u=", yalv);
            print_axis_index(yalv);
            if (ioctl(fd, EVIOCGABS(yalv), &abs_features)) {
                perror("    Get features");
                goto END;
            }
            deadzone_percent = (float)abs_features.flat*100/(float)abs_features.maximum;
            printf("[MIN:%-4d MAX:%-4d DEADZONE:%-3d(=%.2f%%) FUZZ:%d]\n",
                    abs_features.minimum,
                    abs_features.maximum,
                    abs_features.flat,
                    deadzone_percent,
                    abs_features.fuzz);
            abs_features.flat=0;
        }
    }
    ret = 0;
END:
    if (fd != -1) {
        close(fd); 
    }
    return(ret);
}


int set_dead_zone_value(char *dev, int axisindex, int32_t deadzonevalue)
{
    int                  fd = -1;
    uint8_t              abs_bitmask[ABS_MAX/8 + 1];
    float                deadzone_percent;
    struct input_absinfo abs_features;
    int                  ret = 1;
    int                  min_axis, max_axis;

    if ((fd = open(dev, O_RDONLY)) < 0) {
        perror("    Open");
        goto END;
    }

    memset(abs_bitmask, 0, sizeof(abs_bitmask));
    if (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask) < 0) {
        // perror("    Get features mask");
        goto END;
    }
    if (axisindex == -1) {
        min_axis = 0; 
        max_axis = ABS_MAX - 1;
    }
    else {
        if (axisindex >= ABS_MAX) {
            goto END;
        }
        min_axis = axisindex;
        max_axis = axisindex;
    }
    for (axisindex = min_axis ; axisindex <= max_axis ; axisindex++) {
        if (test_bit(axisindex, abs_bitmask)) {
            /* this means that the bit is set in the axes list */
            printf("    %2u=", axisindex);
            print_axis_index(axisindex);

            if(ioctl(fd, EVIOCGABS(axisindex), &abs_features)) {
                // perror("    Get features");
                goto END;
            }
            if (deadzonevalue < abs_features.minimum || deadzonevalue > abs_features.maximum) {
                printf("    Deadzone value must be between %d and %d for this axis, value requested : %d\n",abs_features.minimum,abs_features.maximum,deadzonevalue);
                goto END;
            }
            abs_features.flat=deadzonevalue;
            if(ioctl(fd, EVIOCSABS(axisindex), &abs_features)) {
                perror("    Set features");
                goto END;
            }
            if(ioctl(fd, EVIOCGABS(axisindex), &abs_features)) {
                perror("    Get features");
                goto END;
            }
            deadzone_percent=(float)abs_features.flat*100/(float)abs_features.maximum;
            printf("[MIN:%-4d MAX:%-4d DEADZONE:%-3d(=%.2f%%) FUZZ:%d]\n",
                    abs_features.minimum,
                    abs_features.maximum,
                    abs_features.flat,
                    deadzone_percent,
                    abs_features.fuzz);
        }
    }
    ret = 0;
END:
    if (fd != -1) {
        close(fd);
    }
    return(ret);
}

void help()
{
    printf("Usage :\n\n");
    printf("jdzone [options]\n\n");
    printf("[options] are:  \n");    
    printf("  --help      \n");
    printf("  --verbose                                             Verbose mode.\n");
    printf("  --brief                                               Concise mode.\n");
    printf("  [--dev=/dev/input/<eventn>] --show                    Show the calibration values for the specified joystick.\n");
    printf("  [--dev=/dev/input/<eventn>] --dz=value [--axis=index] Set the deadzone value (for specific axis).\n");
    printf("\n\n");
}

int main (int argc, char **argv)
{
    char          *dev = NULL;
    int            c;
    int32_t        flat;
    int            axisindex = -1;
    bool           setdeadzone=false;
    bool           show=false;
    DIR           *d = NULL;
    struct dirent *dir = NULL;
    int            ret = 1;
    char           ev[128];

    static struct option long_options[] =  {
        {"verbose", no_argument,       &verbose_flag, 1},
        {"brief",   no_argument,       &verbose_flag, 0},
        {"help",    no_argument,       0, 'h'},
        {"dev",     required_argument, 0, 'd'},
        {"dz",      required_argument, 0, 'f'},
        {"axis",    required_argument, 0, 'a'},
        {"show",    no_argument,       0, 's'},
        {0, 0, 0, 0}
    };

    while (1)  {
        int option_index = 0;
        c = getopt_long (argc, argv, "hcnzr:d:f:a:s:", long_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
            case 0:
                if (long_options[option_index].flag != 0)
                    break;
                break;

            case 'a':
                axisindex = atoi(optarg);
                break;

            case 'h':
                help();
                break;

            case 'd':
                dev = optarg;
                break;

            case 'f':
                flat = atoi(optarg);
                setdeadzone=true;
                break;

            case 's':
                show = true;
                break;

            case '?':
                /* getopt_long already printed an error message. */
                break;

            default:
                goto END;
        }
    }
    if (show) {
        if (dev == NULL) {
            d = opendir("/dev/input");
            if (d == NULL) {
                perror("Opendir");
                goto END;
            }
            dir = readdir(d);
            while (dir) {
                if (dir->d_type == DT_CHR) {
                    memset(ev, 0, sizeof(ev));
                    snprintf(ev, sizeof(ev) - 1, "/dev/input/%s", dir->d_name);
                    printf ("Current calibration on %s:\n", ev);
                    show_calibration(ev);
                }
                dir = readdir(d);   
            }
            closedir(d);
            d = NULL;
        }
        else {
            printf ("Current calibration on %s:\n",dev);
            show_calibration(dev);            
        }
    }

    if (setdeadzone) {
        if (dev == NULL) {
            d = opendir("/dev/input");
            if (d == NULL) {
                perror("Opendir");
                goto END;
            }
            dir = readdir(d);
            while (dir) {
                if (dir->d_type == DT_CHR) {
                    memset(ev, 0, sizeof(ev));
                    snprintf(ev, sizeof(ev) - 1, "/dev/input/%s", dir->d_name);
                    printf ("Set deadzone to %d on %s\n", flat, ev);
                    set_dead_zone_value(ev, axisindex, flat);
                }
                dir = readdir(d);   
            }
            closedir(d);
            d = NULL;
        }
        else {
            printf ("Set deadzone to %d on %s\n", flat, dev);
            set_dead_zone_value(dev, axisindex, flat);            
        }
    }
    ret = 0;
END:
    return ret;
}
