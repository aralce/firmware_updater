#include "Update.h"

#if defined(CUSTOM_UPDATE_H_FOR_ESP_IDF_PORT)
#include "esp_spi_flash.h"
#include "esp_ota_ops.h"
#include "esp_image_format.h"
#include <esp_log.h>
#include <HAL_GPIO/HAL_GPIO_singleton.h>
#include <HAL_system/HAL_system_singleton.h>

const char UPDATE_CLASS_TAG[13] = "Update class";

static const char * _err2str(uint8_t _error){
    if(_error == UPDATE_ERROR_OK){
        return ("No Error");
    } else if(_error == UPDATE_ERROR_WRITE){
        return ("Flash Write Failed");
    } else if(_error == UPDATE_ERROR_ERASE){
        return ("Flash Erase Failed");
    } else if(_error == UPDATE_ERROR_READ){
        return ("Flash Read Failed");
    } else if(_error == UPDATE_ERROR_SPACE){
        return ("Not Enough Space");
    } else if(_error == UPDATE_ERROR_SIZE){
        return ("Bad Size Given");
    } else if(_error == UPDATE_ERROR_STREAM){
        return ("Stream Read Timeout");
    } else if(_error == UPDATE_ERROR_MD5){
        return ("MD5 Check Failed");
    } else if(_error == UPDATE_ERROR_MAGIC_BYTE){
        return ("Wrong Magic Byte");
    } else if(_error == UPDATE_ERROR_ACTIVATE){
        return ("Could Not Activate The Firmware");
    } else if(_error == UPDATE_ERROR_NO_PARTITION){
        return ("Partition Could Not be Found");
    } else if(_error == UPDATE_ERROR_BAD_ARGUMENT){
        return ("Bad Argument");
    } else if(_error == UPDATE_ERROR_ABORT){
        return ("Aborted");
    }
    return ("UNKNOWN");
}

static HAL_GPIO_api* gpio = HAL_GPIO_singleton::get_HAL_GPIO_instance();
static HAL_system_api* device = HAL_system_singleton::get_HAL_system_instance();

static bool _partitionIsBootable(const esp_partition_t* partition){
    uint8_t buf[ENCRYPTED_BLOCK_SIZE];
    if(!partition){
        return false;
    }
    // if(!ESP.partitionRead(partition, 0, (uint32_t*)buf, ENCRYPTED_BLOCK_SIZE)) {
    if(!esp_partition_read(partition, 0, (uint32_t*)buf, ENCRYPTED_BLOCK_SIZE) == ESP_OK) {
        return false;
    }

    if(buf[0] != ESP_IMAGE_HEADER_MAGIC) {
        return false;
    }
    return true;
}

bool UpdateClass::_enablePartition(const esp_partition_t* partition){
    if(!partition){
        return false;
    }
    // return ESP.partitionWrite(partition, 0, (uint32_t*) _skipBuffer, ENCRYPTED_BLOCK_SIZE);
    return esp_partition_write(partition, 0, (uint32_t*) _skipBuffer, ENCRYPTED_BLOCK_SIZE) == ESP_OK;
}

UpdateClass::UpdateClass()
: _error(0)
, _buffer(0)
, _bufferLen(0)
, _size(0)
, _progress_callback(NULL)
, _progress(0)
, _paroffset(0)
, _command(U_FLASH)
, _partition(NULL)
{
}

UpdateClass& UpdateClass::onProgress(THandlerFunction_Progress fn) {
    _progress_callback = fn;
    return *this;
}

void UpdateClass::_reset() {
    if (_buffer)
        delete[] _buffer;
    _buffer = 0;
    _bufferLen = 0;
    _progress = 0;
    _size = 0;
    _command = U_FLASH;

    if(_ledPin != -1) {
      gpio->set_pin_state(_ledPin, (HAL_pin_state)!_ledOn);
    //   digitalWrite(_ledPin, !_ledOn); // off
    }
}

bool UpdateClass::canRollBack(){
    if(_buffer){ //Update is running
        return false;
    }
    const esp_partition_t* partition = esp_ota_get_next_update_partition(NULL);
    return _partitionIsBootable(partition);
}

bool UpdateClass::rollBack(){
    if(_buffer){ //Update is running
        return false;
    }
    const esp_partition_t* partition = esp_ota_get_next_update_partition(NULL);
    return _partitionIsBootable(partition) && !esp_ota_set_boot_partition(partition);
}

bool UpdateClass::begin(size_t size, int command, int ledPin, uint8_t ledOn, const char *label) {
    if(_size > 0){
        ESP_LOGW(UPDATE_CLASS_TAG, "already running");
        // log_w("already running");
        return false;
    }

    _ledPin = ledPin;
    _ledOn = !!ledOn; // 0(LOW) or 1(HIGH)

    _reset();
    _error = 0;
    _target_md5 = emptyString;
    _md5 = MD5Builder();

    if(size == 0) {
        _error = UPDATE_ERROR_SIZE;
        return false;
    }

    if (command == U_FLASH) {
        _partition = esp_ota_get_next_update_partition(NULL);
        if(!_partition){
            _error = UPDATE_ERROR_NO_PARTITION;
            return false;
        }
        ESP_LOGD(UPDATE_CLASS_TAG, "OTA Partition: %s", _partition->label);
        // log_d("OTA Partition: %s", _partition->label);
    }
    else if (command == U_SPIFFS) {
        _partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, label);
        _paroffset = 0;
        if(!_partition){
            _partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_FAT, NULL);
            _paroffset = 0x1000;  //Offset for ffat, assuming size is already corrected
            if(!_partition){
               _error = UPDATE_ERROR_NO_PARTITION;
               return false;
            }
        }
    }
    else {
        _error = UPDATE_ERROR_BAD_ARGUMENT;
        ESP_LOGE(UPDATE_CLASS_TAG, "bad command %u", command);
        // log_e("bad command %u", command);
        return false;
    }

    if(size == UPDATE_SIZE_UNKNOWN){
        size = _partition->size;
    } else if(size > _partition->size){
        _error = UPDATE_ERROR_SIZE;
        ESP_LOGE(UPDATE_CLASS_TAG, "too large %u > %u", size, _partition->size);
        // log_e("too large %u > %u", size, _partition->size);
        return false;
    }

    //initialize
    _buffer = (uint8_t*)malloc(SPI_FLASH_SEC_SIZE);
    if(!_buffer){
        ESP_LOGE(UPDATE_CLASS_TAG, "malloc failed");
        // log_e("malloc failed");
        return false;
    }
    _size = size;
    _command = command;
    _md5.begin();
    return true;
}

void UpdateClass::_abort(uint8_t err){
    _reset();
    _error = err;
}

void UpdateClass::abort(){
    _abort(UPDATE_ERROR_ABORT);
}

bool UpdateClass::_writeBuffer(){
    //first bytes of new firmware
    uint8_t skip = 0;
    if(!_progress && _command == U_FLASH){
        //check magic
        if(_buffer[0] != ESP_IMAGE_HEADER_MAGIC){
            _abort(UPDATE_ERROR_MAGIC_BYTE);
            return false;
        }

        //Stash the first 16 bytes of data and set the offset so they are
        //not written at this point so that partially written firmware
        //will not be bootable
        skip = ENCRYPTED_BLOCK_SIZE;
        _skipBuffer = (uint8_t*)malloc(skip);
        if(!_skipBuffer){
            ESP_LOGE(UPDATE_CLASS_TAG, "malloc failed");
            // log_e("malloc failed");
        return false;
        }
        memcpy(_skipBuffer, _buffer, skip);
    }
    if (!_progress && _progress_callback) {
        _progress_callback(0, _size);
    }
    size_t offset = _partition->address + _progress;
    bool block_erase = (_size - _progress >= SPI_FLASH_BLOCK_SIZE) && (offset % SPI_FLASH_BLOCK_SIZE == 0);             // if it's the block boundary, than erase the whole block from here
    bool part_head_sectors = _partition->address % SPI_FLASH_BLOCK_SIZE && offset < (_partition->address / SPI_FLASH_BLOCK_SIZE + 1) * SPI_FLASH_BLOCK_SIZE;    // sector belong to unaligned partition heading block
    bool part_tail_sectors = offset >= (_partition->address + _size) / SPI_FLASH_BLOCK_SIZE * SPI_FLASH_BLOCK_SIZE;     // sector belong to unaligned partition tailing block
    if (block_erase || part_head_sectors || part_tail_sectors){
        // if(!ESP.partitionEraseRange(_partition, _progress, block_erase ? SPI_FLASH_BLOCK_SIZE : SPI_FLASH_SEC_SIZE)){
        if(!esp_partition_erase_range(_partition, _progress, block_erase ? SPI_FLASH_BLOCK_SIZE : SPI_FLASH_SEC_SIZE) == ESP_OK){
            _abort(UPDATE_ERROR_ERASE);
            return false;
        }
    }

    // try to skip empty blocks on unecrypted partitions
    // if ((_partition->encrypted || _chkDataInBlock(_buffer + skip/sizeof(uint32_t), _bufferLen - skip)) && !ESP.partitionWrite(_partition, _progress + skip, (uint32_t*)_buffer + skip/sizeof(uint32_t), _bufferLen - skip)) {
    if ((_partition->encrypted || _chkDataInBlock(_buffer + skip/sizeof(uint32_t), _bufferLen - skip)) && !esp_partition_write(_partition, _progress + skip, (uint32_t*)_buffer + skip/sizeof(uint32_t), _bufferLen - skip) == ESP_OK) {
        _abort(UPDATE_ERROR_WRITE);
        return false;
    }

    //restore magic or md5 will fail
    if(!_progress && _command == U_FLASH){
        _buffer[0] = ESP_IMAGE_HEADER_MAGIC;
    }
    _md5.add(_buffer, _bufferLen);
    _progress += _bufferLen;
    _bufferLen = 0;
    if (_progress_callback) {
        _progress_callback(_progress, _size);
    }
    return true;
}

bool UpdateClass::_verifyHeader(uint8_t data) {
    if(_command == U_FLASH) {
        if(data != ESP_IMAGE_HEADER_MAGIC) {
            _abort(UPDATE_ERROR_MAGIC_BYTE);
            return false;
        }
        return true;
    } else if(_command == U_SPIFFS) {
        return true;
    }
    return false;
}

bool UpdateClass::_verifyEnd() {
    if(_command == U_FLASH) {
        if(!_enablePartition(_partition) || !_partitionIsBootable(_partition)) {
            _abort(UPDATE_ERROR_READ);
            return false;
        }

        if(esp_ota_set_boot_partition(_partition)){
            _abort(UPDATE_ERROR_ACTIVATE);
            return false;
        }
        _reset();
        return true;
    } else if(_command == U_SPIFFS) {
        _reset();
        return true;
    }
    return false;
}

bool UpdateClass::setMD5(const char * expected_md5){
    if(strlen(expected_md5) != 32)
    {
        return false;
    }
    _target_md5 = expected_md5;
    return true;
}

bool UpdateClass::end(bool evenIfRemaining){
    if(hasError() || _size == 0){
        return false;
    }

    if(!isFinished() && !evenIfRemaining){
        ESP_LOGE(UPDATE_CLASS_TAG, "premature end: res:%u, pos:%u/%u\n", getError(), progress(), _size);
        // log_e("premature end: res:%u, pos:%u/%u\n", getError(), progress(), _size);
        _abort(UPDATE_ERROR_ABORT);
        return false;
    }

    if(evenIfRemaining) {
        if(_bufferLen > 0) {
            _writeBuffer();
        }
        _size = progress();
    }

    _md5.calculate();
    if(_target_md5.length()) {
        if(_target_md5 != _md5.toString()){
            _abort(UPDATE_ERROR_MD5);
            return false;
        }
    }

    return _verifyEnd();
}

size_t UpdateClass::write(uint8_t *data, size_t len) {
    if(hasError() || !isRunning()){
        return 0;
    }

    if(len > remaining()){
        _abort(UPDATE_ERROR_SPACE);
        return 0;
    }

    size_t left = len;

    while((_bufferLen + left) > SPI_FLASH_SEC_SIZE) {
        size_t toBuff = SPI_FLASH_SEC_SIZE - _bufferLen;
        memcpy(_buffer + _bufferLen, data + (len - left), toBuff);
        _bufferLen += toBuff;
        if(!_writeBuffer()){
            return len - left;
        }
        left -= toBuff;
    }
    memcpy(_buffer + _bufferLen, data + (len - left), left);
    _bufferLen += left;
    if(_bufferLen == remaining()){
        if(!_writeBuffer()){
            return len - left;
        }
    }
    return len;
}

size_t UpdateClass::writeStream(Stream &data) {
    size_t written = 0;
    size_t toRead = 0;
    int timeout_failures = 0;

    if(hasError() || !isRunning())
        return 0;

    if(!_verifyHeader(data.peek())) {
        _reset();
        return 0;
    }

    if(_ledPin != -1) {
        gpio->set_pin_mode(_ledPin, HAL_pin_mode::OUTPUT_MODE);
        // pinMode(_ledPin, OUTPUT);
    }

    while(remaining()) {
        if(_ledPin != -1) {
            gpio->set_pin_state(_ledPin, (HAL_pin_state)_ledOn);
            // digitalWrite(_ledPin, _ledOn); // Switch LED on
        }
        size_t bytesToRead = SPI_FLASH_SEC_SIZE - _bufferLen;
        if(bytesToRead > remaining()) {
            bytesToRead = remaining();
        }

        /* 
        Init read&timeout counters and try to read, if read failed, increase counter,
        wait 100ms and try to read again. If counter > 300 (30 sec), give up/abort
        */
        toRead = 0;
        timeout_failures = 0;
        while(!toRead) {
            toRead = data.readBytes(_buffer + _bufferLen,  bytesToRead);
            if(toRead == 0) {
                timeout_failures++;
                if (timeout_failures >= 300) {
                    _abort(UPDATE_ERROR_STREAM);
                    return written;
                }
                device->wait_ms_blocked(100);
                // delay(100);
            }
        }

        if(_ledPin != -1) {
            gpio->set_pin_state(_ledPin, (HAL_pin_state)!_ledOn);
            // digitalWrite(_ledPin, !_ledOn); // Switch LED off
        }
        _bufferLen += toRead;
        if((_bufferLen == remaining() || _bufferLen == SPI_FLASH_SEC_SIZE) && !_writeBuffer())
            return written;
        written += toRead;
    }
    return written;
}

void UpdateClass::printError(){
    printf("%s\r\n", _err2str(_error));
}

const char * UpdateClass::errorString(){
    return _err2str(_error);
}

bool UpdateClass::_chkDataInBlock(const uint8_t *data, size_t len) const {
    // check 32-bit aligned blocks only
    if (!len || len % sizeof(uint32_t))
        return true;

    size_t dwl = len / sizeof(uint32_t);

    do {
        if (*(uint32_t*)data ^ 0xffffffff)      // for SPI NOR flash empty blocks are all one's, i.e. filled with 0xff byte
            return true;

        data += sizeof(uint32_t);
    } while (--dwl);
    return false;
}

UpdateClass Update;

#endif //CUSTOM_UPDATE_H_FOR_ESP_IDF_PORT