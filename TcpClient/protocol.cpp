#include "protocol.h"

PDU *mkPDU(uint uiMsgLen)
{
    uint uiPDULen=sizeof(PDU)+uiMsgLen;//因为发的文件都是字符一个字节
    PDU*pdu=(PDU*)malloc(uiPDULen);
    if(nullptr==pdu)
    {
        exit(-1);
    }
    memset(pdu,0,uiPDULen);
    pdu->uiPDULen=uiPDULen;
    pdu->uiMsgLen=uiMsgLen;
    return pdu;
}
