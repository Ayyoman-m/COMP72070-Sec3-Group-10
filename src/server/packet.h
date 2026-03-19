class Packet
{
private:
    
    // HEADER
   
    unsigned char magicNumber;
    unsigned char version;
    unsigned short commandId;
    unsigned short statusCode;
    unsigned int payloadLength;

    
    // BODY
   
    char* payload;

 
   // TAIL

    unsigned short checksum;

public:
    Packet();
    Packet(unsigned short cmd, unsigned short status);
    Packet(const Packet& other);
    Packet& operator=(const Packet& other);
    ~Packet();

    // Setters
    void setCommandId(unsigned short cmd);
    void setStatusCode(unsigned short status);
    void setPayload(const char* data, unsigned int length);

    // Getters
    unsigned char getMagicNumber() const;
    unsigned char getVersion() const;
    unsigned short getCommandId() const;
    unsigned short getStatusCode() const;
    unsigned int getPayloadLength() const;
    char* getPayload() const;
    unsigned short getChecksum() const;



};