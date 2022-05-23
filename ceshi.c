for(int reSendCount = 0;reSendCount<10;reSendCount++)
 {
  int reciverLength = 0;
  int nowReciverLength=0;
  //CRC检验
  unsigned char crcCheck[2] = {'0'};
  unsigned char crcSendCmd[28] = {'\0'};
  memcpy(crcSendCmd,sendCmd+1,28);
  get_crc16(crcSendCmd,28,crcCheck);
  char resultCRC[4] = {'\0'};
  HexToAscii(resultCRC,crcCheck,4);
  memcpy(sendCmd+29,resultCRC,4);
  int sendLength = Write(sendCmd,CMD_LENGTH);
  memcpy(cmdSendMsg,sendCmd,34);
  int reReadCount = 0;
  if(CMD_LENGTH != sendLength)
  {
   UNLOCK_MUTEX(&ListMutex);
   return sendCmdResult;
  }
  do
  {
   SLEEP(200);
   nowReciverLength=Read(reciverCmd+reciverLength,CMD_LENGTH*15-reciverLength);
   reciverLength=reciverLength+nowReciverLength;
   if(reciverLength%CMD_LENGTH == 0)
    break;
   if(nowReciverLength ==0)
    reReadCount++;
   if(reReadCount >5)
    break;
  }while(true);
  memcpy(cmdRecMsg,reciverCmd,CMD_LENGTH*15);
  for(int i=0;i<reciverLength/CMD_LENGTH;i++)
  {
   char checkCmd[CMD_LENGTH] = {0};
   memcpy(checkCmd,reciverCmd+(CMD_LENGTH*i),CMD_LENGTH);
   if((checkCmd[2] == 0X7E) && (checkCmd[3] == 0X7E) && (checkCmd[4] == 0X7E))
   {//为上报事件回复
    reportCmdList.push_back(string(checkCmd)); 
   }
   else if((checkCmd[2] == 0X2A) && (checkCmd[3] == 0X2A) && (checkCmd[4] == 0X2A))
   {//为指令回复不可执行
    sendCmdResult = -1;
   }
   else
   {//为指令回复
    char checkCmdType[10] = {'\0'};
    memcpy(checkCmdType,checkCmd+5,14);
    checkCmdType[3] = checkCmdType[3]-1;
    bool sendCmdTypeResult = true;
    for(int j=0;j<4;j++)
    {
     if(sendCmd[1+j] != checkCmdType[j])
      sendCmdTypeResult = false;
    }
    if(sendCmdTypeResult)
     sendCmdResult = 0;
   }
  }
  if(sendCmdResult == 0)
   break;
 }