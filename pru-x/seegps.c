/*
 * seegps.c -- See all data coming from libgps
 *
 * The source code shows you exactly how to extract GPS data from libgps.
 *
 * RESULT
 *   Each time you call gps_read(), it fills in the gps_data_t struct,
 *   setting bits in .set to indicate which members contain new and
 *   valid data. After the first few reads, you'll get the current fix
 *   each time, no matter how often you call gps_read().
 *
 *   What cpgs displays as "heading" (the direction you're facing) is
 *   actually "track" (the direction in which you've been moving).
 *   Heading is actually provided gps_data_t.attitude, whose validity
 *   is indicated by ATTITUDE_SET (see the code below).
 *
 * I didn't bother writing code to print all the data in gps_data_t, such
 * as which satellites sent the data.
 *
 * For more details on gps_data_t, see /usr/include/gps.h.
 *
 * By Ben Kovitz, September 2015.
 */

#include <gps.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define ONE_SECOND 1000000  // in microseconds

#define NUM_GPS_READS 5  // number of libgps reads to perform

void print_timestamp(timestamp_t timestamp) {
  if (timestamp == 0) {
    printf("no timestamp");
  } else {
    time_t seconds_since_epoch = (time_t)timestamp;

    struct tm *time_struct = localtime(&seconds_since_epoch);
    if (time_struct == NULL) {
      perror("localtime");
      exit(1);
    }

    char timestring[200];
    if (strftime(
        timestring,
        sizeof(timestring),
        "%a, %d %b %Y %T %z",
        time_struct
    ) == 0) {
      perror("strftime");
      exit(1);
    }
    puts(timestring);
  }
}

void print_gps_fix(struct gps_fix_t *fix) {
  if (fix->mode < 2)
    return;
  printf("  timestamp %f +/- %f sec\n", fix->time, fix->ept);
  printf("  latitude %f +/- %f m\n", fix->latitude, fix->epy);
  printf("  longitude %f +/- %f m\n", fix->longitude, fix->epx);
  if (fix->mode >= 3)
    printf("  altitude %f +/- %f m\n", fix->altitude, fix->epv);
  printf("  track %f +/- %f deg\n", fix->track, fix->epd);
  printf("  speed %f +/- %f m/sec\n", fix->speed, fix->eps);
  if (fix->mode >= 3)
    printf("  climb %f +/- %f m/sec\n", fix->climb, fix->epc);
}

void print_devconfig(struct devconfig_t *dc) {
    printf("  path \"%s\"\n", dc->path);
    if (dc->flags & SEEN_GPS)
      puts("    SEEN_GPS");
    if (dc->flags & SEEN_RTCM2)
      puts("    SEEN_RTCM2");
    if (dc->flags & SEEN_RTCM3)
      puts("    SEEN_RTCM3");
    if (dc->flags & SEEN_AIS)
      puts("    SEEN_AIS");
    printf("    driver \"%s\"\n", dc->driver);
    printf("    subtype \"%s\"\n", dc->subtype);
    printf("    activated %f\n", dc->activated);
    printf("    %d/%d/%c\n", dc->baudrate, dc->stopbits, dc->parity);
    printf("    cycle %f, mincycle %f\n", dc->cycle, dc->mincycle);
    printf("    driver_mode %s\n", dc->driver_mode ? "native" : "non-native");
}

void print_attitude(struct attitude_t *att) {
  printf("  heading %f\n", att->heading);
  printf("  pitch %f\n", att->pitch);
  printf("  roll %f\n", att->roll);
  printf("  yaw %f\n", att->yaw);
  printf("  dip %f\n", att->dip);
  printf("  mag_len %f\n", att->mag_len);
  printf("  mag_x %f\n", att->mag_x);
  printf("  mag_y %f\n", att->mag_y);
  printf("  mag_z %f\n", att->mag_z);
  printf("  acc_len %f\n", att->acc_len);
  printf("  acc_x %f\n", att->acc_x);
  printf("  acc_y %f\n", att->acc_y);
  printf("  acc_z %f\n", att->acc_z);
  printf("  gyro_x %f\n", att->gyro_x);
  printf("  gyro_y %f\n", att->gyro_y);
  printf("  temp %f\n", att->temp);
  printf("  depth %f\n", att->depth);
  printf("  mag_st %c\n", att->mag_st);
  printf("  pitch_st %c\n", att->pitch_st);
  printf("  roll_st %c\n", att->roll_st);
  printf("  yaw_st %c\n", att->yaw_st);
}

void one_gps_cycle(struct gps_data_t *gps_data) {
  int err;

  if ((err = gps_waiting(gps_data, ONE_SECOND)) == 0) {
    if (errno == 0) {
      puts("timeout");
      return;
    } else {
      fprintf(stderr, gps_errstr(err));
      exit(1);
    }
  }

  if ((err = gps_read(gps_data)) < 0) {
    perror("gps_read");
    exit(1);
  }

  print_timestamp(gps_data->online);

  switch (gps_data->status) {
    case STATUS_NO_FIX:
      puts("No fix");
      break;

    case STATUS_FIX:
      puts("Have fix without DGPS");
      print_gps_fix(&gps_data->fix);
      break;

    case STATUS_DGPS_FIX:
      puts("Have fix with DGPS");
      print_gps_fix(&gps_data->fix);
      break;

    default:
      printf("status = %d\n", gps_data->status);
  }

  if (gps_data->set & ERROR_SET) 
    printf("ERROR_SET  error: %s\n", gps_data->error);

  if (gps_data->set & LOGMESSAGE_SET)
    puts("LOGMESSAGE_SET");

  if (gps_data->set & POLICY_SET)
    puts("POLICY_SET");

  if (gps_data->set & VERSION_SET)
    printf("VERSION_SET  \"%s\" \"%s\" %d.%d \"%s\"\n",
      gps_data->version.release,
      gps_data->version.rev,
      gps_data->version.proto_major,
      gps_data->version.proto_minor,
      gps_data->version.remote);

  if (gps_data->set & GST_SET) {
    puts("GST_SET  (pseudorange errors)");
    printf("  utctime %g\n", gps_data->gst.utctime);
    printf("  rms_deviation %g\n", gps_data->gst.rms_deviation);
    printf("  smajor_deviation %g\n", gps_data->gst.smajor_deviation);
    printf("  sminor_deviation %g\n", gps_data->gst.sminor_deviation);
    printf("  smajor_orientation %g\n", gps_data->gst.smajor_orientation);
    printf("  lat_err_deviation %g\n", gps_data->gst.lat_err_deviation);
    printf("  lon_err_deviation %g\n", gps_data->gst.lon_err_deviation);
    printf("  alt_err_deviation %g\n", gps_data->gst.alt_err_deviation);
  }

  if (gps_data->set & SUBFRAME_SET)
    puts("SUBFRAME_SET");

  if (gps_data->set & PACKET_SET)
    puts("PACKET_SET");

  if (gps_data->set & AIS_SET)
    puts("AIS_SET");

  if (gps_data->set & RTCM3_SET)
    puts("RTCM3_SET");

  if (gps_data->set & RTCM2_SET)
    puts("RTCM2_SET");

  if (gps_data->set & DEVICEID_SET)
    puts("DEVICEID_SET");

  if (gps_data->set & DEVICELIST_SET) {
    printf("DEVICELIST_SET\n  ");
    print_timestamp(gps_data->devices.time);
    for (int i = 0; i < gps_data->devices.ndevices; i++)
      print_devconfig(&gps_data->devices.list[i]);
  }

  if (gps_data->set & DEVICE_SET)
    puts("DEVICE_SET");

  if (gps_data->set & CLIMBERR_SET)
    puts("CLIMBERR_SET");

  if (gps_data->set & TRACKERR_SET)
    puts("TRACKERR_SET");

  if (gps_data->set & SPEEDERR_SET)
    puts("SPEEDERR_SET");

  if (gps_data->set & SATELLITE_SET)
    puts("SATELLITE_SET");

  if (gps_data->set & ATTITUDE_SET) {
    puts("ATTITUDE_SET");
    print_attitude(&gps_data->attitude);
  }

  if (gps_data->set & VERR_SET)
    puts("VERR_SET");

  if (gps_data->set & HERR_SET)
    puts("HERR_SET");

  if (gps_data->set & DOP_SET)
    puts("DOP_SET");

  if (gps_data->set & MODE_SET)
    puts("MODE_SET");

  if (gps_data->set & STATUS_SET)
    puts("STATUS_SET");

  if (gps_data->set & CLIMB_SET)
    puts("CLIMB_SET");

  if (gps_data->set & TRACK_SET)
    puts("TRACK_SET");

  if (gps_data->set & SPEED_SET)
    puts("SPEED_SET");

  if (gps_data->set & ALTITUDE_SET)
    puts("ALTITUDE_SET");

  if (gps_data->set & LATLON_SET)
    puts("LATLON_SET");

  if (gps_data->set & TIMERR_SET)
    puts("TIMERR_SET");

  if (gps_data->set & TIME_SET)
    puts("TIME_SET");

  if (gps_data->set & ONLINE_SET) {
    printf("ONLINE_SET  ");
    print_timestamp(gps_data->online);
  }

  //printf("0x%08x\n", (unsigned)gps_data->set);
  putchar('\n');
}

int main() {
  struct gps_data_t gps_data;
  int err;

  if ((err = gps_open("localhost", DEFAULT_GPSD_PORT, &gps_data)) != 0) {
    fprintf(stderr, gps_errstr(err));
    exit(1);
  }

  gps_stream(&gps_data, WATCH_ENABLE, NULL);

  for (int i = 0; i < NUM_GPS_READS; i++)
    one_gps_cycle(&gps_data);


  return 0;
}
