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

#include <linux/input.h>

/* this macro is used to tell if "bit" is set in "array"
 * it selects a byte from the array, and does a boolean AND 
 * operation with a byte that only has the relevant bit set. 
 * eg. to check for the 12th bit, we do (array[1] & 1<<4)
 */
#define test_bit(bit, array)    (array[bit/8] & (1<<(bit%8)))

static int verbose_flag;

int showcalibration(char *evdev)
{
    int fd = -1;
    uint8_t abs_bitmask[ABS_MAX/8 + 1];
    int yalv;
    float deadzone_percent;
    struct input_absinfo abs_features;

    if ((fd = open(evdev, O_RDONLY)) < 0) {
        return 1;
    }

    memset(abs_bitmask, 0, sizeof(abs_bitmask));
    if (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask) < 0) {
        return 1;
    }

    printf("Supported absolute axes:\n");

    for (yalv = 0; yalv < ABS_MAX; yalv++) {
        if (test_bit(yalv, abs_bitmask)) {
            /* this means that the bit is set in the axes list */
            printf("  Absolute axis 0x%02x (%d)", yalv, yalv);
            switch ( yalv) {
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
            if (ioctl(fd, EVIOCGABS(yalv), &abs_features)) {
                return 1;
            }
            deadzone_percent = (float)abs_features.flat*100/(float)abs_features.maximum;
            printf("(min: %d, max: %d, flatness: %d (=%.2f%%), fuzz: %d)\n",
                    abs_features.minimum,
                    abs_features.maximum,
                    abs_features.flat,
                    deadzone_percent,
                    abs_features.fuzz);
                    abs_features.flat=0;
        }
    }
    close(fd);
    return(0);
}


int setdeadzonevalue(char *evdev, int axisindex, int32_t deadzonevalue)
{
    int fd = -1;
    uint8_t abs_bitmask[ABS_MAX/8 + 1];
    float deadzone_percent;
    struct input_absinfo abs_features;

    if ((fd = open(evdev, O_RDONLY)) < 0) {
        return 1;
    }

    memset(abs_bitmask, 0, sizeof(abs_bitmask));
    if (ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask) < 0) {
        return 1;
    }

    if (axisindex < ABS_MAX) {
        if (test_bit(axisindex, abs_bitmask)) {
            /* this means that the bit is set in the axes list */
            printf("  absolute axis 0x%02x (%d)", axisindex, axisindex);
            switch (axisindex)  {
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
            if(ioctl(fd, EVIOCGABS(axisindex), &abs_features)) {
                return 1;
            }
            if ( deadzonevalue < abs_features.minimum || deadzonevalue > abs_features.maximum ) {
                printf("Deadzone value must be between %d and %d for this axis, value requested : %d\n",abs_features.minimum,abs_features.maximum,deadzonevalue);
            }
            printf("Setting deadzone value to : %d\n",deadzonevalue);
            abs_features.flat=deadzonevalue;
            if(ioctl(fd, EVIOCSABS(axisindex), &abs_features)) {
                return 1;
            }
            if(ioctl(fd, EVIOCGABS(axisindex), &abs_features)) {
                return 1;
            }
            deadzone_percent=(float)abs_features.flat*100/(float)abs_features.maximum;
            printf("(min: %d, max: %d, flatness: %d (=%.2f%%), fuzz: %d)\n",
                    abs_features.minimum,
                    abs_features.maximum,
                    abs_features.flat,
                    deadzone_percent,
                    abs_features.fuzz);
        }
    }
    close(fd);
    return(0);
}

void help()
{
    printf("Usage :\n\n");
    printf("jinfo [options]\n\n");
    printf("[options] are:  \n");    
    printf("  --help      \n");
    printf("  --verbose  verbose mode.\n");
    printf("  --brief    concise mode.\n");
    printf("  --showcalibration=/dev/input/<eventn>  show the calibration values for the specified joystick.\n");
    printf("  --evdev=/dev/input/<eventn> --deadzone=value [--axis=index] set the deadzone value (for specific axis).\n");
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
                printf ("Axis index to deal with: %d\n", axisindex);
                break;

            case 'h':
                help();
                break;

            case 'd':
                evdevice=optarg;
                printf ("Event device file: %s\n", evdevice);
                break;

            case 'f':
                flat=atoi(optarg);
                setdeadzone=true;
                printf ("New dead zone value: %d\n", flat);
                break;

            case 's':
                evdevice=optarg;
                showcalibration(evdevice);
                break;

            case '?':
                /* getopt_long already printed an error message. */
                break;

            default:
                return 1;
                break;
        }
    }

    if (setdeadzone) {
        if (evdevice == NULL){
            printf ( "You must specify the event device for your joystick/wheel\n" );
            return 1;
        }
        else {
            printf ( "Trying to set axis %d deadzone to: %d\n",axisindex,flat );
            setdeadzonevalue(evdevice,axisindex,flat);
        }
    }
    return 0;

}
