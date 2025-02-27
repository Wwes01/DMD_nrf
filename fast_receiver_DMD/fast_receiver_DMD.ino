extern "C" {
#include <correct.h>
}


#define FAST_LIB

#ifdef FAST_LIB
#include <DueAdcFast.h>
DueAdcFast DueAdcF(1024);
void ADC_Handler() {
  DueAdcF.adcHandler();
}

#endif

const char* WANTED_PAYLOAD = "Hello World!";
const uint8_t WANTED_LENGTH = 12;

// const char* WANTED_PAYLOAD = "Testing right now!";
// const uint8_t WANTED_LENGTH = 18;
uint8_t BYTES_RS = 4;
char WANTED_RS[18];

#define DEBUG
#define PERIOD 500

#ifndef DEBUG
#define PAYLOAD_COUNTS 500
#endif

#define PD A0 // PD: Photodiode

#define MANCHESTER_BITS_LENGTH 16
#define THRESHOLD_FACTOR_PERIOD 1.5
#define THRESHOLD_FACTOR_ERROR  2.5

// Constant add: should be negative for far away, positive for nearby
const int AVG_CONSTANT_ADD = 3;

const int PREAMBLE_LENGTH = (PERIOD * 3 + 1);
int preamble_detect[PREAMBLE_LENGTH];
int head = 0;

const int PREAMBLE_PATTERN = 4;
const int preamble[PREAMBLE_PATTERN] = {1, 3, 3, 1};
int pattern_detect[PREAMBLE_PATTERN - 2];
int head_pattern = 0;

uint16_t avg = 0;
uint32_t total = 0;
uint8_t length = 0;

char decoded[1000];
uint16_t decoded_head = 0;
uint8_t bit_count = 0;

uint32_t last_time;
uint32_t this_time;

uint16_t last_sample;
uint16_t this_sample;

bool is_high = !(PREAMBLE_PATTERN%2);

// Copies to avoid conflicts
uint16_t print_avg;
uint8_t print_length;
char print_decoded[1000];
char print_message[1000];
char print_rs[18];

uint16_t new_avg;
uint8_t new_length;
char new_decoded[1000];
volatile bool new_message = false;

const long expected_ms_for_all_messages = PERIOD * 500 * 500 / 1000;
const long automatic_display_results_ms = expected_ms_for_all_messages * 1.5;

struct ctx
{
  ctx() {counter=0;}
  uint32_t counter;
};

ctx ctx_preamble;

// End needed for hardware timers

#ifndef DEBUG
typedef struct {
  uint16_t avg;
  uint8_t length;
  char * payload;
} payload_error;

#define DEFAULT_PAYLOAD_ERROR(_avg, _length, _payload)                   \
{                                                                        \
    .avg    = _avg,                                                      \
    .length = _length,                                                   \
    .payload = _payload                                                  \
}

payload_error payloads[PAYLOAD_COUNTS];
uint16_t payloads_head = 0;
#endif


void read_preamble(double factor_0, double factor_1);
inline void register_bit(bool bit_state);
int read_incoming(double factor_0, double factor_1, int incoming_reads);
void reset();
void fast_samples(void);
inline bool ccf();
inline uint8_t count_errors(const char* m1, char* m2);
inline void sample(void);

const int PIN_RED = 12;
const int PIN_GREEN = 11;
const int PIN_BLUE = 10;

/*
* Setup function. 
* Starts tc0.
*/
void setup() {
  Serial.begin(115200);
  Serial.println(" ---");
  analogReadResolution(12);
  pinMode(A1, OUTPUT);
  #ifdef FAST_LIB
  DueAdcF.EnablePin(PD);
  DueAdcF.Start1Mhz();
  #else
  pinMode(PD, INPUT);
  #endif

  char temp[18 + WANTED_LENGTH] = { 0 };
  correct_reed_solomon* rs_code = correct_reed_solomon_create(correct_rs_primitive_polynomial_8_4_3_2_0, 1, 1, 4);
  correct_reed_solomon_encode(rs_code, (const uint8_t*) WANTED_PAYLOAD, WANTED_LENGTH, (uint8_t*) temp);

  for(int i = WANTED_LENGTH; i < 18 + WANTED_LENGTH; i++) {
    WANTED_RS[i - WANTED_LENGTH] = temp[i];
    if(temp[i] == '\0') {
      break;
    }
  }

  // Try out sample speed real quick
  int start = micros();
  int total_test_value = 0;
  int min_test_value = 4096;
  int max_test_value = 0;
  const int amount_of_test_samples = 100000;
  for (int i = 0; i < amount_of_test_samples; i++) {
    sample();
    total_test_value += this_sample;
    min_test_value = this_sample < min_test_value ? this_sample : min_test_value;
    max_test_value = this_sample > max_test_value ? this_sample : max_test_value;
  }
  int took = micros() - start;
  Serial.print("One sample takes about ");
  Serial.print(took / amount_of_test_samples);
  Serial.print("us. Average: ");
  Serial.print(total_test_value / amount_of_test_samples);
  Serial.print(" in range [");
  Serial.print(min_test_value);
  Serial.print(", ");
  Serial.print(max_test_value);
  Serial.println("]");

  correct_reed_solomon_destroy(rs_code);
  
  Serial.print("Period: ");
  Serial.print(PERIOD);
  Serial.print("  | exp. payload: ");
  Serial.print(WANTED_PAYLOAD);
  Serial.print("  | exp. length: ");
  Serial.print(WANTED_LENGTH);
  Serial.print("  | exp. rs: ");
  Serial.println(WANTED_RS);

  #ifndef DEBUG
  Serial.print("Automatically showing after not messages for ");
  Serial.print(automatic_display_results_ms / 1000);
  Serial.println(" seconds.");
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  #endif

  memset(preamble_detect, 0, sizeof(preamble_detect));
  memset(decoded, 0, sizeof(decoded));
}

uint8_t state = 0;
uint8_t nr_periods;
bool last_state = false;
bool temp_state;
uint8_t counter = 0;
uint32_t took;

unsigned long last_message_received_at_ms = 0;

void read_preamble(double factor_0, double factor_1) {
  memset(preamble_detect, 0, sizeof(preamble_detect));
  total = 0;
  
  while(true) {
    long time_since_last_msg = millis() - last_message_received_at_ms;
    #ifndef DEBUG
      if(payloads_head == PAYLOAD_COUNTS) {
        reboot_record();
      } else if (Serial.available()) {
        String command = Serial.readString();
        command.trim();
        if(command.equals("end")) { 
          reboot_record();
        }
      } else if (time_since_last_msg > automatic_display_results_ms) {
        last_message_received_at_ms = millis();
        if (payloads_head > 0) reboot_record();
      }
    #else
    if (time_since_last_msg > 5000) {
      last_message_received_at_ms = millis();
      Serial.print("Alive. Average: ");
      Serial.print(avg);
      int min = 10000, max = 0;
      for (int i = 0; i < PREAMBLE_LENGTH; i++) {
        min = preamble_detect[i] < min ? preamble_detect[i] : min;
        max = preamble_detect[i] > max ? preamble_detect[i] : max;
      }
      Serial.print(" in range [");
      Serial.print(min);
      Serial.print(", ");
      Serial.print(max);
      Serial.println("]");
    }
    #endif
    total -= preamble_detect[head];
    sample();
    preamble_detect[head] = this_sample;
    total += preamble_detect[head];
    avg = (total / PREAMBLE_LENGTH) + AVG_CONSTANT_ADD;

    head = (head + 1) % PREAMBLE_LENGTH;

    temp_state = (this_sample < avg);
    if(temp_state != last_state) {
      last_state = temp_state;
      took = this_time - last_time;
      last_time = this_time;

      nr_periods = (((double) took) / PERIOD) + 0.5 ;
      pattern_detect[head_pattern] = nr_periods;
      head_pattern = (head_pattern + 1) % (PREAMBLE_PATTERN - 2);
      
      int temp = 0;
      for(int i = 0; i < PREAMBLE_PATTERN - 2; i++) {
        temp += (pattern_detect[(head_pattern + i) % (PREAMBLE_PATTERN - 2)]) == preamble[i+1];
      }
      if(temp == PREAMBLE_PATTERN - 2 
          && temp_state == (PREAMBLE_PATTERN%2)) {
        break;
      } 
    }
  }

  total -= preamble_detect[head];
  sample();
  preamble_detect[head] = this_sample;
  total += preamble_detect[head];
  avg = total / PREAMBLE_LENGTH + AVG_CONSTANT_ADD;

  head = (head + 1) % PREAMBLE_LENGTH;

  
  temp_state = (this_sample <= avg);
  if(temp_state != last_sample) {
    last_sample = temp_state;
    took = this_time - last_time;
    last_time = this_time;

    nr_periods = took / PERIOD;
    
    if(nr_periods > preamble[counter] - factor_0 && nr_periods < preamble[counter] + factor_1) {
      return;
    } else {
      counter = 0;
      read_preamble(factor_0, factor_1);
    }
  }
}



inline void sample(void) {
  #ifdef FAST_LIB
  this_sample = DueAdcF.ReadAnalogPin(PD);
  #else
  this_sample = analogRead(PD);
  #endif
  this_time = micros();
}

inline void register_bit(bool bit_state) {
  decoded[decoded_head] = decoded[decoded_head] ^ (bit_state << (7 - bit_count)); // Add opposite of preamble end
  bit_count    = (bit_count + 1) % 8;
  decoded_head = (decoded_head + (bit_count == 0));
}

bool uneven;
uint32_t sampling[5000];
uint16_t sample_head = 0;

int read_incoming(double factor_0, double factor_1, int incoming_reads) {
  // 01 -> 1
  // 10 -> 0
  while(incoming_reads > 0) {
    sample();

    if((this_sample <= avg) == is_high) {
      took = this_time - last_time;
      last_time = this_time;
      if(took < (factor_0 * PERIOD)) {
        // 1 symbol
        if(uneven) {
          // Uneven == Edge of bit
          register_bit(is_high);
        } // Else is middle of bit, incoming_reads decr and invert is_high, is already done
        incoming_reads--;
        uneven = !uneven;
      } else if (took < (factor_1 * PERIOD)) {
        // 2 symbols
        register_bit(!is_high);
        incoming_reads -= 2;
      } else {
        // ILLEGAL: Do not add anything to decoded, 
        // since adding a 0 leads to less cascading errors
        // Serial.println("Illegal state: 3+ same symbols in a row!");
        incoming_reads -= 3;
      }
      is_high = !is_high;
    }

  }
  return incoming_reads;
}

void fast_samples(void) {
  read_preamble(0.5, 1.5);

  int reads = 0;
  uneven = true;
  if(took >= (THRESHOLD_FACTOR_PERIOD * PERIOD) && took < (THRESHOLD_FACTOR_ERROR * PERIOD)) {
    uneven = false;
  }
  reads = read_incoming(THRESHOLD_FACTOR_PERIOD, THRESHOLD_FACTOR_ERROR, MANCHESTER_BITS_LENGTH - 1 + reads);
  length = decoded[0];
  // length = 12;
  decoded[0] = 0;
  decoded_head = 0;
  bit_count = 0;
  reads = read_incoming(THRESHOLD_FACTOR_PERIOD, THRESHOLD_FACTOR_ERROR, (length + BYTES_RS) * 2 * 8 );
  last_message_received_at_ms = millis();
}

void loop() {
  // Read package
  fast_samples();

  // Reset preamble variables and copy relevant ones
  new_message = false;
  total = 0;
  head = 0;
  print_avg = avg;
  avg = 0;
  memset(preamble_detect, 0, sizeof(preamble_detect));
  
  // Print result
  #ifdef DEBUG
  process_result(print_avg, length, decoded, true); // false = dont print
  #else
  char * new_decoded = (char *) malloc(length+BYTES_RS * sizeof(char) + 1);
  for(int i = 0; i < length+BYTES_RS; i++) {
      new_decoded[i] = decoded[i];
  }
  new_decoded[length+BYTES_RS] = '\0';
  payloads[payloads_head++] = DEFAULT_PAYLOAD_ERROR(print_avg, length, new_decoded);
  #endif

  // Reset rest of variables
  length = 0;
  decoded_head = 0;
  bit_count = 0;
  memset(decoded, 0, sizeof(decoded));
  is_high = preamble[PREAMBLE_PATTERN - 1];
}


uint32_t process_result(uint16_t payload_avg, uint8_t payload_length, char* decoded, bool print) {
  static int correct;
  // Seperate reed-solomon parity bytes for printing.
  for(int i = 0; i < BYTES_RS; i++) {
    print_rs[i] = decoded[i + payload_length];
  }

  // Check and correction with Reed-Solomon
  correct_reed_solomon* rs_code = correct_reed_solomon_create(correct_rs_primitive_polynomial_8_4_3_2_0, 1, 1, 4);
  // correct = correct_reed_solomon_decode(rs_code, (const uint8_t*) decoded, payload_length + BYTES_RS, (uint8_t*) print_message);
  
  correct = payload_length;
  uint8_t newly_encoded[270];
  memset(newly_encoded, 0, sizeof(newly_encoded));
  int rs_bytes = correct_reed_solomon_encode(rs_code, (const uint8_t*) decoded, (size_t) payload_length, newly_encoded);
  for (int i = 0; i < BYTES_RS; i++) {
    if (newly_encoded[payload_length + i] != decoded[payload_length + i]) {
      correct = -1;
      break;
    }
  }

  // If incorrect, copy over print_decoded anyway
  // if(correct == -1) {
  for(int i = 0; i < payload_length; i++) {
    print_message[i] = decoded[i];
  }
  // }
  print_message[payload_length] = '\0';

  // Count errors
  static uint32_t err = 0;
  err = __builtin_popcount(WANTED_LENGTH ^ payload_length);
  err += count_errors(WANTED_PAYLOAD, print_message);
  err += count_errors(WANTED_RS, print_rs);
  if (payload_length < WANTED_LENGTH) {
    err += abs(WANTED_LENGTH - payload_length)*8;
  }

  if (print) {
    if (correct != -1)  Serial.print("-VALID MESSAGE-");
    else                Serial.print("invalid message");
    Serial.print("  Avg: ");
    Serial.print(payload_avg);
    Serial.print("  |  Length: ");
    Serial.print(payload_length);
    Serial.print("  | Encoded:  '");
    Serial.print(decoded);
    Serial.print("'  |  Payload: '");
    Serial.print(print_message);
    Serial.print("'  |  rs: ");
    Serial.print(print_rs);
    Serial.print("  | Correct:  ");
    Serial.print(correct);
    Serial.print("  | Error:  ");
    Serial.println(err);
  }

  if (correct != -1 && print_message[0] == '#') {
    static int lastR, lastG, lastB;

    #define diff(a,b) (a < b ? (b - a) : (a - b))
    // if (diff(lastR, print_message[1]) < 5 && diff(lastG, print_message[2]) < 5 && diff(lastB, print_message[3]) < 5) {
      analogWrite(PIN_RED, print_message[1]);
      analogWrite(PIN_GREEN, print_message[2]);
      analogWrite(PIN_BLUE, print_message[3]);
      if (print) Serial.print(" Changed RGB: #");
    // } else {
    //   if (print) Serial.print(" Skipping RGB change: #");
    // }
    if (print) {
      Serial.print((int)print_message[1]);
      Serial.print(", ");
      Serial.print((int)print_message[2]);
      Serial.print(", ");
      Serial.print((int)print_message[3]);
      Serial.println();
    }

    lastR = print_message[1];
    lastG = print_message[2];
    lastB = print_message[3];
  }

  correct_reed_solomon_destroy(rs_code);  
  return err;
}
/*
* Counts the bit differences in two char*.
*/
inline uint8_t count_errors(const char* m1, char* m2) {
  uint8_t counter = 0;
  for(int i = 0; i < WANTED_LENGTH; i++) {
    if(m1[i] == '\0' || m2[i] == '\0') break;
    counter += __builtin_popcount(m1[i] ^ m2[i]);
  }
  return counter;
}

#ifndef DEBUG
/*
* Stop tc0, tc1, tc2, to start processing.
* Processes and prints data on the last PAYLOAD_COUNTS packets.
*/
void reboot_record(void) {
  uint32_t incorrect_bits = 0;
  uint16_t packets_lost = PAYLOAD_COUNTS - payloads_head;

  const uint32_t BITS_PER_MESSAGE = 8*WANTED_LENGTH + (MANCHESTER_BITS_LENGTH/2) + 8*BYTES_RS;

  const bool long_print = false;
  static int record_run = 0;
  record_run += 1;

  uint32_t total_average = 0;
  for(int i = 0; i < payloads_head; i++) {
    incorrect_bits += process_result(payloads[i].avg, payloads[i].length, payloads[i].payload, long_print);
    total_average += payloads[i].avg;
    free(payloads[i].payload);
    payloads[i].payload = NULL;
    payloads[i].avg = 0;
    payloads[i].length = 0;
  }

  double per = (1.0 - (payloads_head / (double) PAYLOAD_COUNTS)) * 100.0;
  double ber_minus = incorrect_bits / (double)(payloads_head * BITS_PER_MESSAGE) * 100.0;
  double ber_plus = (incorrect_bits + (packets_lost * BITS_PER_MESSAGE)) / (double)(PAYLOAD_COUNTS * BITS_PER_MESSAGE) * 100.0;
  double average_average = total_average / (double)payloads_head;

  if (long_print) {
    Serial.println();
    Serial.print("Run: ");
    Serial.println(record_run);
    Serial.print("Packets received: ");
    Serial.println(payloads_head);
    Serial.print("Packets lost: ");
    Serial.println(packets_lost);

    Serial.print("Amount incorrect bits: ");
    Serial.println(incorrect_bits);

    Serial.print("PER: ");
    Serial.println(per);
    Serial.print("BER minus lost: ");
    Serial.println(ber_minus);
    Serial.print("BER plus lost: ");
    Serial.println(ber_plus);
  }

  Serial.print("[Run ");
  Serial.print(record_run);
  Serial.print(" (avg: ");
  Serial.print(average_average);
  Serial.print(")] ");
  Serial.print(payloads_head);
  Serial.print(", ");
  Serial.print(per);
  Serial.print(", ");
  Serial.print(ber_minus);
  Serial.print(", ");
  Serial.print(ber_plus);
  Serial.println();

  payloads_head = 0;
}
#endif