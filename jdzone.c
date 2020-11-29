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
            printf("x: ");
            break;

        case ABS_Y :
            printf("y: ");
            break;

        case ABS_Z :
            printf("z: ");
            break;

        case ABS_RX :
            printf("x rate: ");
            break;

        case ABS_RY :
            printf("y rate: ");
            break;

        case ABS_RZ :
            printf("z rate: ");
            break;

        case ABS_THROTTLE :
            printf("throttle: ");
            break;

        case ABS_RUDDER :
            printf("rudder: ");
            break;

        case ABS_WHEEL :
            printf("wheel: ");
            break;

        case ABS_GAS :
            printf("accel: ");
            break;

        case ABS_BRAKE :
            printf("brake: ");
            break;

        case ABS_HAT0X :
            printf("hat zero,x: ");
            break;

        case ABS_HAT0Y :
            printf("hat zero,y: ");
            break;

        case ABS_HAT1X :
            printf("hat one,x: ");
            break;

        case ABS_HAT1Y :
            printf("hat one,y: ");
            break;

        case ABS_HAT2X :
            printf("hat two,x: ");
            break;

        case ABS_HAT2Y :
            printf("hat two,y: ");
            break;

        case ABS_HAT3X :
            printf("hat three,x: ");
            break;

        case ABS_HAT3Y :
            printf("hat three,y: ");
            break;

        case ABS_PRESSURE :
            printf("pressure:");
            break;

        case ABS_DISTANCE :
            printf("distance: ");
            break;

        case ABS_TILT_X :
            printf("tilt,x: ");
            break;

        case ABS_TILT_Y :
            printf("tilt,y: ");
            break;

        case ABS_MISC :
            printf("misc: ");
            break;

        default:
            printf("unk: ");
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
        perror("open");
        goto END;
    }

    memset(abs_bitmask, 0, sizeof(abs_bitmask));
    if (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask) < 0) {
        perror("get features mask");
        goto END;
    }

    printf("Supported absolute axes:\n");
    for (yalv = 0; yalv < ABS_MAX; yalv++) {
        if (test_bit(yalv, abs_bitmask)) {
            printf("absolute axis 0x%02x (%d)", yalv, yalv);
            print_axis_index(yalv);
            if (ioctl(fd, EVIOCGABS(yalv), &abs_features)) {
                perror("get features");
                goto END;
            }
            deadzone_percent = (float)abs_features.flat*100/(float)abs_features.maximum;
            printf("(min: %d, max: %d, deadzone: %d (=%.2f%%), fuzz: %d)\n",
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


int set_dead_zone_value(char *evdev, int axisindex, int32_t deadzonevalue)
{
    int                  fd = -1;
    uint8_t              abs_bitmask[ABS_MAX/8 + 1];
    float                deadzone_percent;
    struct input_absinfo abs_features;
    int                  ret = 1;

    if ((fd = open(evdev, O_RDONLY)) < 0) {
        perror("open");
        goto END;
    }

    memset(abs_bitmask, 0, sizeof(abs_bitmask));
    if (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask) < 0) {
        perror("get features mask");
        goto END;
    }
    if (axisindex < ABS_MAX) {
        if (test_bit(axisindex, abs_bitmask)) {
            /* this means that the bit is set in the axes list */
            printf("absolute axis 0x%02x (%d)", axisindex, axisindex);
            print_axis_index(axisindex);

            if(ioctl(fd, EVIOCGABS(axisindex), &abs_features)) {
                perror("get features");
                goto END;
            }
            if ( deadzonevalue < abs_features.minimum || deadzonevalue > abs_features.maximum ) {
                printf("deadzone value must be between %d and %d for this axis, value requested : %d\n",abs_features.minimum,abs_features.maximum,deadzonevalue);
                goto END;
            }
            printf("Setting deadzone value to : %d\n",deadzonevalue);
            abs_features.flat=deadzonevalue;
            if(ioctl(fd, EVIOCSABS(axisindex), &abs_features)) {
                perror("set features");
                goto END;
            }
            if(ioctl(fd, EVIOCGABS(axisindex), &abs_features)) {
                perror("get features");
                goto END;
            }
            deadzone_percent=(float)abs_features.flat*100/(float)abs_features.maximum;
            printf("(min: %d, max: %d, deadzone: %d (=%.2f%%), fuzz: %d)\n",
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
    printf("  --verbose  verbose mode.\n");
    printf("  --brief    concise mode.\n");
    printf("  --showcalibration=/dev/input/<eventn>  show the calibration values for the specified joystick.\n");
    printf("  [--evdev=/dev/input/<eventn>] --deadzone=value [--axis=index] set the deadzone value (for specific axis).\n");
    printf("\n\n");
}

int main (int argc, char **argv)
{
    char *evdevice;
    int c;
    int32_t flat;
    int axisindex;
    bool setdeadzone;
    axisindex = 0;
    setdeadzone = false;
    evdevice = NULL;
    DIR           *d = NULL;
    struct dirent *dir = NULL;
    int ret = 1;

    static struct option long_options[] =  {
        {"verbose", no_argument,       &verbose_flag, 1},
        {"brief",   no_argument,       &verbose_flag, 0},
        {"help",   no_argument,       0, 'h'},
        {"evdev",  required_argument, 0, 'd'},
        {"deadzone",  required_argument, 0, 'f'},
        {"axis",  required_argument, 0, 'a'},
        {"showcalibration",  required_argument, 0, 's'},
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
                /* If this option set a flag, do nothing else now. */
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;

            case 'a':
                axisindex=atoi(optarg);
                printf ("axis index to deal with: %d\n", axisindex);
                break;

            case 'h':
                help();
                break;

            case 'd':
                evdevice=optarg;
                printf ("event device file: %s\n", evdevice);
                break;

            case 'f':
                flat=atoi(optarg);
                setdeadzone=true;
                printf ("new dead zone value: %d\n", flat);
                break;

            case 's':
                evdevice=optarg;
                show_calibration(evdevice);
                break;

            case '?':
                /* getopt_long already printed an error message. */
                break;

            default:
                goto END;
        }
    }

    if (setdeadzone) {
        if (evdevice == NULL) {
            d = opendir("/dev/input");
            if (d == NULL) {
                perror("opendir");
                goto END;
            }
            dir = readdir(d);
            while (dir) {
                if (dir->d_type == DT_REG) {
                    printf ("trying to set axis %d deadzone to: %d on %s\n",axisindex, flat, dir->d_name);
                    set_dead_zone_value(dir->d_name,axisindex,flat);
                }
                dir = readdir(d);   
            }
            closedir(d);
            d = NULL;
        }
        else {
            printf ("trying to set axis %d deadzone to: %d on %s\n",axisindex,flat, evdevice);
            set_dead_zone_value(evdevice,axisindex,flat);            
        }
    }
    ret = 0;
END:
    return ret;
}
