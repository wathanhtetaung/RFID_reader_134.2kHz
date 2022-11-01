// (c) Michael Schoeffler 2018, http://www.mschoeffler.de
#include <SoftwareSerial.h>
#include <PriUint64.h> // https://github.com/yoursunny/PriUint64 

const int BUFFER_SIZE = 13; // RFID DATA FRAME FORMAT: 1byte head (value: 2), 10byte data (2byte version + 8byte tag), 2byte checksum, 1byte tail (value: 3)
const int DATA_SIZE = 10; // 10byte data (2byte version + 8byte tag)
// const int DATA_VERSION_SIZE = 2; // 2byte version (actual meaning of these two bytes may vary)
const int DATA_TAG_SIZE = 10; // 8byte tag
const int CHECKSUM_SIZE = 1; // 2byte checksum/ 1 byte for 134.2kHz application

SoftwareSerial ssrfid = SoftwareSerial(7,9); 

uint8_t buffer[BUFFER_SIZE]; // used to store an incoming data frame. originally active line
// char buffer[BUFFER_SIZE]; // used to store an incoming data frame. added by Wathan
int buffer_index = 0;

void setup() {
 Serial.begin(9600); 
 
 ssrfid.begin(9600);
 ssrfid.listen(); 
 
 Serial.println("INIT DONE");
}

void loop() {
  if (ssrfid.available() > 0){
    bool call_extract_tag = false;
    
    int ssvalue = ssrfid.read(); // read 
    if (ssvalue == -1) { // no data was read
      return;
    }

    if (ssvalue == 0xAA) { // RDM630/RDM6300 found a tag => tag incoming 
      buffer_index = 0;
    } else if (ssvalue == 0xBB) { // tag has been fully transmitted       
      call_extract_tag = true; // extract tag at the end of the function call
    }

    if (buffer_index >= BUFFER_SIZE) { // checking for a buffer overflow (It's very unlikely that an buffer overflow comes up!)
      Serial.println("Error: Buffer overflow detected!");
      return;
    }
    
    buffer[buffer_index++] = ssvalue; // everything is alright => co  py current value to buffer

    if (call_extract_tag == true) {
      if (buffer_index == BUFFER_SIZE) {
        unsigned tag = extract_tag();
      } else { // something is wrong... start again looking for preamble (value: 2)
        Serial.println("call_extract was not true");
        buffer_index = 0;
        return;
      }
    }    
  }    
}

unsigned extract_tag() {
    uint8_t msg_head = buffer[0];
    uint8_t *msg_data = buffer + 1; // 10 byte => data contains 2byte version + 8byte tag ... originally 'buffer + 1'
    uint8_t *msg_data_7B = buffer + 4;
    // uint8_t *msg_data_version = msg_data;
    uint8_t *msg_data_tag = msg_data; // originally 'msg_date + 2'
    uint8_t *msg_checksum = buffer[11]; // 2 byte .. originally 'buffer + 11'. buffer[1] for 134.2kHz
    uint8_t msg_tail = buffer[12]; // buffer[13] for 125kHz. buffer[12] for 134.2kHz application.

    // print message that was sent from RDM630/RDM6300
    Serial.println("--------");

    Serial.print("Message-Head: ");
    Serial.println(msg_head,HEX);

    /*
    Serial.println("Message-Data (HEX): ");
    for (int i = 0; i < DATA_VERSION_SIZE; ++i) {
      Serial.print(char(msg_data_version[i]),HEX);
    }
    Serial.println(" (version)");
    */
    
    for (int i = 0; i < DATA_TAG_SIZE; ++i) 
    {
      
      Serial.print((msg_data_tag[i]),HEX);
    }
    Serial.println(" (tag)");

    long msg_checksum_st;
    Serial.print("Message-Checksum (HEX): ");
    // msg_checksum_st = strtol(msg_checksum, NULL, 16);
    Serial.print(buffer[11],HEX);
    Serial.print("\n");

    uint8_t hex[10];
    uint64_t tag;
    Serial.print("tag buffer (HEX): ");
    for (int i = 1; i <= DATA_TAG_SIZE; ++i) 
    {
      hex[i-1] = uint8_t(buffer[i]);
      Serial.print((hex[i-1]),HEX);
      tag=hex[i-1];
    }

    /* ... */
    tag = hex[3];
    tag = (tag << 8) | hex[4];
    tag = (tag << 8) | hex[5];
    tag = (tag << 8) | hex[6];
    tag = (tag << 8) | hex[7];
    tag = (tag << 8) | hex[8];
    tag = (tag << 8) | hex[9];
 
    /* ...
    for (int i = 0; i < CHECKSUM_SIZE; ++i) {
      Serial.print(char(msg_checksum[i]),HEX);
    }
    */
  
    
    Serial.println("");

    Serial.print("Message-Tail: ");
    Serial.println(msg_tail,HEX);

    Serial.println("--");

    // long tag = hexstr_to_value(hex, DATA_TAG_SIZE);
    // long tag;
    Serial.print("Extracted Tag: ");
    // tag = strtol(tag, NULL, 16);
    // tag = hexstr_to_value(msg_data_7B, DATA_TAG_SIZE);
    // Serial.println(tag);
    Serial.println(PriUint64<DEC>(tag));
    /* ...
    Serial.print(hex[3],HEX);
    Serial.print(hex[4],HEX);
    Serial.print(hex[5],HEX);
    Serial.print(hex[6],HEX);
    Serial.print(hex[7],HEX);
    Serial.print(hex[8],HEX);
    Serial.println(hex[9],HEX);
    */
    
    // Serial.println(tag);

    long checksum = 0;
    /* ...
    // xore the tag_id and validate checksum
    for (uint8_t i = 0; i < 16; i += 8)
      checksum ^= ((tag >> i) & 0xFF);
    // if (checksum)
    //  return 0;
    */

    byte xorTemp;
    xorTemp = byte(hex[0]);
    // Serial.println(xorTemp);
    for(int i = 1; i < DATA_SIZE; i++)
    {
      xorTemp ^= byte (hex[i]);
      // Serial.println(xorTemp);
      // delay(50);
    }
    // Serial.println(xorTemp,HEX);
    checksum = xorTemp;
   
    /* .....
    long checksum = 0;
    for (int i = 0; i < DATA_SIZE; i+= CHECKSUM_SIZE) {
      long val = hexstr_to_value(msg_data + i, CHECKSUM_SIZE);
      checksum ^= val;
    }
    */
    
    Serial.print("Calculated Checksum (HEX): ");
    Serial.print(checksum,HEX);
    /* ...
    if (checksum == hexstr_to_value(msg_checksum, CHECKSUM_SIZE)) { // compare calculated checksum to retrieved checksum
      Serial.print(" (OK)"); // calculated checksum corresponds to transmitted checksum!
    } else {
      Serial.print(" (NOT OK)"); // checksums do not match
    }
    */
    if (checksum == msg_checksum) 
    { // compare calculated checksum to retrieved checksum
      Serial.print(" (OK)"); // calculated checksum corresponds to transmitted checksum!
    } 
    else 
    {
      Serial.print(" (NOT OK)"); // checksums do not match
    }

    Serial.println("");
    Serial.println("--------");

    return tag;
}

long hexstr_to_value(char *str, unsigned int length) { // converts a hexadecimal value (encoded as ASCII string) to a numeric value
  char* copy = malloc((sizeof(char) * length) + 1); 
  memcpy(copy, str, sizeof(char) * length);
  copy[length] = '\0'; 
  // the variable "copy" is a copy of the parameter "str". "copy" has an additional '\0' element to make sure that "str" is null-terminated.
  long value = strtol(copy, NULL, 16);  // strtol converts a null-terminated string to a long value
  free(copy); // clean up 
  return value;
}
