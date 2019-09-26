#include <stdio.h>
#include <stdlib.h>
#include "mojing.pb-c.h"

int main()
{
    uint8_t *buf = NULL;
    char message[] = "00111111111111111111111111111100";
    ProtoBuf__GetSDCardInfoResponse  notify = PROTO_BUF__GET_SDCARD_INFO_RESPONSE__INIT;

    ProtoBuf__GetSDCardInfoResponse  *msg = NULL;

    notify.message = message;
    notify.code = 0;
    notify.sdstat = 1;
    notify.sdcardsize = 2;
    notify.sdcardavailablesize = 3;
    notify.sdcardusedsize = 4;

    int len = proto_buf__get_sdcard_info_response__get_packed_size(&notify);
    printf("package size:%d\n", len);
    buf = (uint8_t *)malloc(len);
    proto_buf__get_sdcard_info_response__pack(&notify, buf);

    //upack
    msg = proto_buf__get_sdcard_info_response__unpack(NULL, len, buf);
    printf("message:%s\ncode:%d\nsdstat:%d\nsdcardsize:%d\nsdcardavailablesize:%d\nsdcardusedsize:%d\n",
            msg->message, msg->code, msg->sdstat, msg->sdcardsize, msg->sdcardavailablesize,
            msg->sdcardusedsize);

    proto_buf__get_sdcard_info_response__free_unpacked(msg, NULL);

    return 0;
}
