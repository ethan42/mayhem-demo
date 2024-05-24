#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define URL "http://api:8000/location"

#define CIN(x)  ((x) >= '0' && (x) <= '9')
#define CTN(x)  ((x) - '0')


// Taken from LWGPS (https://github.com/ms-rtos/lwgps/) 
// Floating point overflow. 
double parse_float_number(const char* text) {
  double value = 0.0, power = 1.0;
  int sign = 1;
  if (text == NULL)
    return value;

  for (; text != NULL && *text == ' '; ++text) {} /* Strip leading spaces */
  if (*text == '-') { /* Check sign */
    sign = -1;
    ++text;
  }
  while (CIN(*text)) { /* Convert main part */
    value = value * (double)10 + CTN(*text);
    ++text;
  }
  if (*text == '.') { /* Skip the dot character */
    ++text;
  }
  while (CIN(*text)) { /* Get the power */
    value = value * (double)10 + CTN(*text);
    power *= (double)10;
    ++text;
  }
  return sign * value / power;
}


// Function to parse a NMEA line and extract latitude and longitude
void parseLatLon(char* line, double* time, double* latitude, double* longitude) {
  int i = 0;
  char* token;

  token = strtok(line, ",");

  // Check message is of assumed message type for our receiver
  if (strncmp(token, "$GPRMC", 6) == 0) {
    // Read pos status field. 
    token = strtok(NULL, ",");
    *time = parse_float_number(token);
    token = strtok(NULL, ","); // pos_status field. 
    if (token != NULL && token[0] != 'A') {
      printf("Skipping record with invalid position.\n");
      return;
    }
  }
  else if (strncmp(token, "$GPGGA", 6) == 0) {
    token = strtok(NULL, ",");
    *time = parse_float_number(token);
    // There is no pos status field; continue on. 
  }
  else {
    printf("Unsupported format line: %s\n", line);
    return;
  }


  while (token != NULL) {
    switch (i) {
    case 1: // latitude
      *latitude = parse_float_number(token) / 100.0;
      break;
    case 2: // latitude direction (N/S)
      if (token[0] == 'S')
        *latitude = -(*latitude);
      break;
    case 3: // longitude
      *longitude = parse_float_number(token) / 100.0;
      break;
    case 4: // longitude direction (E/W)
      if (token[0] == 'W')
        *longitude = -(*longitude);
      break;
    }
    token = strtok(NULL, ",");
    i++;
  }
}

// Function to upload position data to API server
void upload_position(double latitude, double longitude)
{
  char command[256];
  snprintf(command, sizeof(command),
    "curl -X POST -H \"Content-Type: application/json\" -d '{\"latitude\": %.6f, \"longitude\": %.6f}' %s",
    latitude, longitude, URL);
#ifdef DEBUG
  printf("Command: %s\n", command);
#endif
  system(command);
}

// Damn vulnerable C stack exhaustion
void stack_exhaustion() {
  char buff[0x1000];
  while (1) {
    stack_exhaustion();
  }
}

void damn_vulnerable_c(char* line, int latitude, int longitude) {

  // A bit of computation on our inputs. 
  if (latitude < 0) {
    latitude = -latitude;
  }
  if (longitude < 0) {
    longitude = -longitude;
  }

  // integer overflow 0x7FFFFFFF+1=0
  // 0x7FFFFFFF+2 = 1
  // Volatile keyword ensures variable not optimized away since
  // it's never used past this calculation. 
  volatile int size1 = latitude + longitude;

  // integer underflow 0-1=-1
  volatile int size2 = latitude - longitude;

  // Divide by zero. 
  volatile int size3 = latitude / longitude;

  // Stack exhaustion
  if (size1 == 1000)
    stack_exhaustion();

  char* buff1 = (char*)malloc(strlen(line) + size1);

  strcpy(buff1, line);  // Heap overflow

  char OOBR = buff1[size1]; // Out of bounds read
  buff1[size1] = 'a';       // Out of bound write

  free(buff1);

  if (size1 / 2 == 0) {
    free(buff1);   //double free	
  }
  else {
    if (size1 / 3 == 0) {
      buff1[0] = 'a';     //use after free
    }
  }

  // Memory leak of buf2
  char* buf2 = (char*)malloc(size2);

}

int main(int argc, char* argv[])
{
  double latitude = 0.0, longitude = 0.0, time = 0.0;

  if (argc != 2)
  {
    printf("Usage: %s <nmea_data_file>\n", argv[0]);
    return 1;
  }
  const char* GPS_FILE_PATH = argv[1];

  FILE* file = fopen(GPS_FILE_PATH, "r");
  if (file == NULL)
  {
    perror("Error opening GPS file");
    return 1;
  }

  char line[256];
  char unparsed_line[256];

  // Buffer overflow when line > 256
  while (fgets(line, sizeof(line), file))
  {
    latitude = 0.0;
    longitude = 0.0;
    time = 0.0;
    parseLatLon(line, &time, &latitude, &longitude);

    // Example vulnerable C 
    // Ported from https://github.com/hardik05/Damn_Vulnerable_C_Program/blob/master/imgRead.c
    damn_vulnerable_c(line, (int)latitude, (int)longitude);
    printf("Uploading Latitude: %f, Longitude: %f\n", latitude, longitude);
    upload_position(latitude, longitude);
  }

  fclose(file);
  return 0;
}
