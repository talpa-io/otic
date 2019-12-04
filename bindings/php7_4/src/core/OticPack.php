<?php
/**
 * Created by PhpStorm.
 * User: talpaadmin
 * Date: 02.12.19
 * Time: 11:54
 */

include 'OticExceptions.php';

$ffi = FFI::cdef(
    "
        // config.h
        #define OTIC_VERSION_MAJOR 1
        #define OTIC_VERSION_MINOR 0
        #define OTIC_VERSION_PATCH 0

        #define OTIC_PACK_NO_COMPRESSION 0
        // base.h
        
        #define OTIC_BASE_CACHE_SIZE 12000
        #define OTIC_ZSTD_COMPRESSION_LEVEL 7
        #define OTIC_TS_MULTIPLICATOR 10000
        #define OTIC_ENTRY_STRING_SIZE 32
        #define OTIC_PACK_CACHE_SIZE 255
        #define OTIC_MAGIC_SIZE 4
        #define PTR_M 31
       
        typedef enum
        {
            OTIC_TYPE_NULL,
            OTIC_TYPE_EMPTY_STRING,
            OTIC_TYPE_INT32_NEG,
            OTIC_TYPE_INT32_POS,
            OTIC_TYPE_DOUBLE,
            OTIC_TYPE_MIN1_FLOAT,
            OTIC_TYPE_MIN2_FLOAT,
            OTIC_TYPE_MIN3_FLOAT,
            OTIC_TYPE_FLOAT,
            OTIC_TYPE_MED_DOUBLE,
            OTIC_TYPE_STRING,
            OTIC_TYPE_UNMODIFIED,
            OTIC_TYPE_RAWBUFFER,
            OTIC_TYPE_SET_TIMESTAMP,
            OTIC_TYPE_SHIFT_TIMESTAMP,
            OTIC_TYPE_FILE_VERSION,
            OTIC_TYPE_NAME_ASSIGN,
            OTIC_TYPE_EOF,
            OTIC_TYPE_METADATA,
            OTIC_TYPE_DATA
        } otic_types_e;
        typedef enum
        {
            OTIC_ERROR_NONE,
            OTIC_ERROR_INVALID_POINTER,
            OTIC_ERROR_BUFFER_OVERFLOW,
            OTIC_ERROR_INVALID_TIMESTAMP,
            OTIC_ERROR_ENTRY_INSERTION_FAILURE,
            OTIC_ERROR_ZSTD,
            OTIC_ERROR_FLUSH_FAILED,
            OTIC_ERROR_EOF,
            OTIC_ERROR_INVALID_FILE,
            OTIC_ERROR_DATA_CORRUPTED,
            OTIC_ERROR_VERSION_UNSUPPORTED,
            OTIC_ERROR_ROW_COUNT_MISMATCH,
            OTIC_ERROR_INVALID_ARGUMENT,
            OTIC_ERROR_AT_INVALID_STATE,
            OTIC_ERROR_ALLOCATION_FAILURE,
        } otic_errors_e;
        typedef enum
        {
            OTIC_STATE_OPENED,
            OTIC_STATE_ON_ERROR,
            OTIC_STATE_CLOSED
        } otic_state_e;
        
        typedef enum otic_features_e otic_features_e;
        typedef enum otic_meta_type_e otic_meta_type_e;
        typedef struct otic_header_t otic_header_t;
        typedef struct otic_meta_head_t otic_meta_head_t;
        
        typedef struct otic_meta_data_t;
        typedef struct otic_payload_t otic_payload_t;
        
        typedef struct otic_base_t otic_base_t;
        struct otic_base_t
        {
            uint8_t cache[12000];
            uint8_t* top;
            uint64_t timestamp_start;
            uint64_t timestamp_current;
            otic_errors_e error;
            otic_state_e state;
            size_t rowCounter;
        };
    
        typedef enum {
            OTIC_CHANNEL_TYPE_SENSOR,
            OTIC_CHANNEL_TYPE_BINARY
        } channel_type_e;
        typedef struct 
        {
            char* ptr;
            size_t size;
        } otic_str_t;
        
        void            otic_base_init(otic_base_t* base) __attribute__((nonnull(1)));
        void            otic_base_setError(otic_base_t *base, otic_errors_e error) __attribute__((nonnull(1)));
        otic_errors_e   otic_base_getError(otic_base_t *base) __attribute__((nonnull(1)));
        void            otic_base_setState(otic_base_t* base, otic_state_e state) __attribute__((nonnull(1)));
        otic_state_e    otic_base_getState(otic_base_t* base) __attribute__((nonnull(1)));
        void            otic_base_close(otic_base_t* base) __attribute__((nonnull(1)));
        
        uint8_t         leb128_encode_unsigned(uint32_t value, uint8_t* restrict dest) __attribute__((nonnull(2)));
        uint8_t         leb128_decode_unsigned(const uint8_t* restrict encoded_values, uint32_t* restrict value) __attribute__((nonnull(1, 2)));
        uint8_t         leb128_encode_signed(int64_t value, uint8_t* restrict dest) __attribute__((nonnull(2)));
        uint8_t         leb128_decode_signed(const uint8_t* restrict encoded_values, int64_t* restrict value) __attribute__((nonnull(1, 2)));
        
        otic_str_t*     otic_setStr(const char* ptr);
        void            otic_freeStr(otic_str_t* oticStr) __attribute__((nonnull(1)));
        void            otic_updateStr(otic_str_t* oticStr, const char* ptr) __attribute__((nonnull(1)));
        
        // pack.h
        
        #define ZSTD_OUT_SIZE 12000
        #define OTIC_PACK_CACHE_TOP_LIMIT 255
        typedef struct otic_entry_t otic_entry_t;
        struct otic_entry_t
        {
            uint32_t index;
            char* name;
            struct
            {
                uint32_t int_value;
                double double_value;
                otic_str_t string_value;
            } last_value;
            otic_types_e type;
            otic_entry_t* next;
        };
        typedef struct otic_pack_t otic_pack_t;
        typedef struct ZSTD_CCtx ZSTD_CCtx; 
        typedef struct
        {
            otic_base_t base;
            ZSTD_CCtx* cCtx;
            otic_entry_t* cache[255];
            uint16_t totalEntries;
            uint8_t ztd_out[12000];
            const uint8_t* threshold;
            struct
            {
                otic_pack_t* parent;
                channel_type_e channelType;
                uint8_t channelId;
            } info;
        } otic_pack_channel_t;

        struct otic_pack_t
        {
            otic_pack_channel_t** channels;
            uint8_t totalChannels;
            uint8_t (*flusher)(uint8_t *, size_t, void*);
            void* data;
            otic_errors_e error;
            otic_state_e state;
        };
        
        uint8_t otic_pack_channel_init(
                otic_pack_channel_t* channel,
                uint8_t id,
                channel_type_e channelType,
                otic_pack_t* parent
                ) __attribute__((nonnull(1)));
        uint8_t otic_pack_channel_close(otic_pack_channel_t* channel) __attribute__((nonnull(1)));
        uint8_t otic_pack_channel_inject_i(
                otic_pack_channel_t* channel,
                double timestamp,
                const char* sensorName,
                const char* sensorUnit,
                uint32_t value
                ) __attribute__((nonnull(1)));
        uint8_t otic_pack_channel_inject_i_neg(
                otic_pack_channel_t* channel,
                double timestamp,
                const char* sensorName,
                const char* sensorUnit,
                uint32_t value
                ) __attribute__((nonnull(1)));
        uint8_t otic_pack_channel_inject_d(
                otic_pack_channel_t* channel,
                double timestamp,
                const char* sensorName,
                const char* unit,
                double value
                ) __attribute__((nonnull(1)));
        uint8_t otic_pack_channel_inject_s(
                otic_pack_channel_t* channel,
                double timestamp,
                const char* sensorName,
                const char* unit,
                const char* value
                ) __attribute__((nonnull(1)));
        uint8_t otic_pack_channel_inject_n(
                otic_pack_channel_t* channel,
                double timestamp,
                const char* sensorName,
                const char* unit
                ) __attribute__((nonnull(1)));
        uint8_t otic_pack_channel_inject_b(
                otic_pack_channel_t* channel,
                double timestamp,
                const char* sensorName,
                const char* unit,
                uint8_t* buffer,
                size_t size
                ) __attribute__((nonnull(1)));
        uint8_t otic_pack_channel_flush(otic_pack_channel_t* channel) __attribute__((nonnull(1)));
      
        uint8_t otic_pack_init(otic_pack_t* oticPack, uint8_t(*flusher)(uint8_t*, size_t, void*), void* data) __attribute__((nonnull(1)));
        otic_pack_channel_t*    otic_pack_defineChannel(otic_pack_t* oticPack, channel_type_e channelType, uint8_t id,
                                                        otic_features_e features) __attribute__((warn_unused_result)) __attribute__((nonnull(1)));
        uint8_t                 otic_pack_closeChannel(otic_pack_t* oticPackBase, uint8_t id) __attribute__((nonnull(1)));
        uint8_t                 otic_pack_flush(otic_pack_t* oticPack) __attribute__((nonnull(1)));
        void                    otic_pack_close(otic_pack_t* oticPack) __attribute__((nonnull(1)));        
    ",
    "../../ext2/libotic.so.1.0.0"
);

class OticPackChannel
{
    const CHANNEL_TYPE_SENSOR = 0;
    const CHANNEL_TYPE_BINARY = 1;

    private $channel;
    private $packer;

    public function __construct(&$packer, $channelId, $channelType, $channelFeatures)
    {
        if ($channelId > 255 || $channelId < 0)
            throw new OticException(OticException::OTIC_ERROR_INVALID_ARGUMENT, "Invalid ChannelID!");
        if ($channelType != 1 && $channelType != 0)
            throw new OticException(OticException::OTIC_ERROR_INVALID_ARGUMENT, "Invalid ChannelType!");
        $this->packer = $packer;
        global $ffi;
        $this->channel = $ffi->otic_pack_defineChannel(FFI::addr($this->packer), $channelType, $channelId, $channelFeatures);
        if ($this->packer->error != 0)
            throw new OticException($this->packer->error);
    }

    public function inject($timestamp, $sensorName, $sensorUnit, $value = null)
    {
        global $ffi;
        if ($value === null)
            $ffi->otic_pack_channel_inject_n($this->channel, $timestamp, $sensorName, $sensorUnit);
        elseif (is_numeric($value)) {
            if (strstr($value, '.') === false) {
                if ($value >= 0)
                    $ffi->otic_pack_channel_inject_i($this->channel, $timestamp, $sensorName, $sensorUnit, $value);
                else
                    $ffi->otic_pack_channel_inject_ineg($this->channel, (double)$timestamp, $sensorName, $sensorUnit, -$value);
            } else {
                $ffi->otic_pack_channel_inject_d($this->channel, (double)$timestamp, $sensorName, $sensorUnit, $value);
            }
        } else {
            $ffi->otic_pack_channel_inject_s($this->channel, (double)$timestamp, $sensorUnit, $sensorUnit, $value);
        }
        if ($this->packer->error != 0)
            throw new Error("Inject Error. Error Code: $this->packer->error\n");
    }

    public function flush()
    {
        global $ffi;
        $ffi->otic_pack_channel_flush($this->channel);
        if ($this->packer->error != 0)
            throw new Error("Inject Error. Error Code: $this->packer->error\n");
    }
}

$ffi3 = FFI::cdef("
    typedef struct FILE FILE; 
    FILE* fopen(const char* fileName, const char* mode); 
    size_t fwrite(const void*, size_t, size_t nmemb, FILE*);
    int fflush(FILE* stream);
    ", "libc.so.6");

class OticPack
{
    private $data;
    private $packer;
    private $ffi2;
    public function __construct($fileName)
    {
        global $ffi;
        $this->packer = $ffi->new("otic_pack_t");
        $this->ffi2 = FFI::cdef("typedef struct FILE FILE;int fflush(FILE* stream); FILE* fopen(const char* fileName, const char* mode); size_t fwrite(const void*, size_t, size_t, FILE*);", "libc.so.6");
        $this->data = $this->ffi2->fopen($fileName, "wb");
        $ffi->otic_pack_init(FFI::addr($this->packer), 'OticPack::flusher', $this->data);
        $channel = $ffi->otic_pack_defineChannel(FFI::addr($this->packer), 0, 1, 0);
        $ffi->otic_pack_channel_inject_s($channel, 12323, "sensor1", "sensorUnit1", "Some string");
        $ffi->otic_pack_channel_flush($channel);
        var_dump($channel->base);
    }

    public function defineChannel($channelId, $channelType, $channelFeatures)
    {
        if (!is_numeric($channelId) || !is_numeric($channelType) || !is_numeric($channelFeatures))
            throw new OticException(OticException::OTIC_ERROR_INVALID_ARGUMENT);
        return new OticPackChannel($this->packer, $channelType, $channelId, $channelFeatures);
    }

    protected function flusher($content, $size, $data)
    {
        global $ffi3;
        echo "Size: $size\n";
        $ffi3->fwrite($content, 1, $size, $data);
    }

    public function flush()
    {
        global $ffi;
        $ffi->otic_pack_flush(FFI::addr($this->packer));
        if ($this->packer->error != 0)
            throw new Error("Inject Error. Error Code: $this->packer->error\n");
    }

    public function closeChannel($channelId)
    {
        global $ffi;
        if ($channelId > 255 || $channelId < 0)
            throw new Error("Invalid Channel ID!");
        $ffi->otic_pack_closeChannel(FFI::addr($this->packer), $channelId);
        if ($this->packer->error != 0)
            throw new Error("Inject Error. Error Code: $this->packer->error\n");
    }

    public function __destruct()
    {
        global $ffi;
        $ffi->otic_pack_close(FFI::addr($this->packer));
    }
}

$packer = new OticPack("test.otic");
//$channel = $packer->defineChannel(1, OticPackChannel::CHANNEL_TYPE_SENSOR, 0);
//$channel->inject(1223, "sensor1", "unit1", "Some String");
//$channel->inject(1223, "sensor2", "unit3", "Some Other String");
//for ($i = 0; $i < 5; ++$i)
//{
//    $channel->inject($i, "sensor1", "unit1", 1234);
//}