#include "socket.h"

//M20150401 : Typing Error
//#define SOCK_ANY_PORT_NUM  0xC000;
#define SOCK_ANY_PORT_NUM  0xC000

static uint16_t sock_any_port = SOCK_ANY_PORT_NUM;
static uint16_t sock_io_mode = 0;
static uint16_t sock_is_sending = 0;

static uint16_t sock_remained_size[_WIZCHIP_SOCK_NUM_] = {0,0,};

//M20150601 : For extern decleation
//static uint8_t  sock_pack_info[_WIZCHIP_SOCK_NUM_] = {0,};
uint8_t  sock_pack_info[_WIZCHIP_SOCK_NUM_] = {0,};
//


#define CHECK_SOCKNUM()   \
   do{                    \
      if(sn > _WIZCHIP_SOCK_NUM_) return SOCKERR_SOCKNUM;   \
   }while(0);             \

#define CHECK_SOCKMODE(mode)  \
   do{                     \
      if((getSn_MR(sn) & 0x0F) != mode) return SOCKERR_SOCKMODE;  \
   }while(0);              \

#define CHECK_SOCKINIT()   \
   do{                     \
      if((getSn_SR(sn) != SOCK_INIT)) return SOCKERR_SOCKINIT; \
   }while(0);              \

#define CHECK_SOCKDATA()   \
   do{                     \
      if(len == 0) return SOCKERR_DATALEN;   \
   }while(0);              \



int8_t socket(uint8_t sn, uint8_t protocol, uint16_t port, uint8_t flag)
{
	CHECK_SOCKNUM();
	switch(protocol)
	{
      case Sn_MR_TCP :
         {
            //M20150601 : Fixed the warning - taddr will never be NULL
		    /*
            uint8_t taddr[4];
            getSIPR(taddr);
            */
            uint32_t taddr;
            getSIPR((uint8_t*)&taddr);
            if(taddr == 0) return SOCKERR_SOCKINIT;
         }
      case Sn_MR_UDP :
      case Sn_MR_MACRAW :
	   case Sn_MR_IPRAW :
         break;

      default :
         return SOCKERR_SOCKMODE;
	}
	//M20150601 : For SF_TCP_ALIGN & W5300
	//if((flag & 0x06) != 0) return SOCKERR_SOCKFLAG;
	if((flag & 0x04) != 0) return SOCKERR_SOCKFLAG;

	if(flag != 0)
	{
   	switch(protocol)
   	{
   	   case Sn_MR_TCP:
   		  //M20150601 :  For SF_TCP_ALIGN & W5300
            if((flag & (SF_TCP_NODELAY|SF_IO_NONBLOCK))==0) return SOCKERR_SOCKFLAG;

   	      break;
   	   case Sn_MR_UDP:
   	      if(flag & SF_IGMP_VER2)
   	      {
   	         if((flag & SF_MULTI_ENABLE)==0) return SOCKERR_SOCKFLAG;
   	      }
            if(flag & SF_UNI_BLOCK)
            {
               if((flag & SF_MULTI_ENABLE) == 0) return SOCKERR_SOCKFLAG;
            }
   	      break;

   	   default:
   	      break;
   	}
   }
	close(sn);

	setSn_MR(sn, (protocol | (flag & 0xF0)));

	if(!port)
	{
	   port = sock_any_port++;
	   if(sock_any_port == 0xFFF0) sock_any_port = SOCK_ANY_PORT_NUM;
	}
   setSn_PORT(sn,port);
   setSn_CR(sn,Sn_CR_OPEN);
   while(getSn_CR(sn));
   //A20150401 : For release the previous sock_io_mode
   sock_io_mode &= ~(1 <<sn);
   //
	sock_io_mode |= ((flag & SF_IO_NONBLOCK) << sn);
   sock_is_sending &= ~(1<<sn);
   sock_remained_size[sn] = 0;
   //M20150601 : repalce 0 with PACK_COMPLETED
   //sock_pack_info[sn] = 0;
   sock_pack_info[sn] = PACK_COMPLETED;
   //
   while(getSn_SR(sn) == SOCK_CLOSED);
   return (int8_t)sn;
}

int8_t close(uint8_t sn)
{
	CHECK_SOCKNUM();
//A20160426 : Applied the erratum 1 of W5300
	setSn_CR(sn,Sn_CR_CLOSE);
   /* wait to process the command... */
	while( getSn_CR(sn) );
	/* clear all interrupt of the socket. */
	setSn_IR(sn, 0xFF);
	//A20150401 : Release the sock_io_mode of socket n.
	sock_io_mode &= ~(1<<sn);
	//
	sock_is_sending &= ~(1<<sn);
	sock_remained_size[sn] = 0;
	sock_pack_info[sn] = 0;
	while(getSn_SR(sn) != SOCK_CLOSED);
	return SOCK_OK;
}

int8_t listen(uint8_t sn)
{
	CHECK_SOCKNUM();
   CHECK_SOCKMODE(Sn_MR_TCP);
	CHECK_SOCKINIT();
	setSn_CR(sn,Sn_CR_LISTEN);
	while(getSn_CR(sn));
   while(getSn_SR(sn) != SOCK_LISTEN)
   {
         close(sn);
         return SOCKERR_SOCKCLOSED;
   }
   return SOCK_OK;
}


int8_t connect(uint8_t sn, uint8_t * addr, uint16_t port)
{
   CHECK_SOCKNUM();
   CHECK_SOCKMODE(Sn_MR_TCP);
   CHECK_SOCKINIT();
   //M20140501 : For avoiding fatal error on memory align mismatched
   //if( *((uint32_t*)addr) == 0xFFFFFFFF || *((uint32_t*)addr) == 0) return SOCKERR_IPINVALID;
   {
      uint32_t taddr;
      taddr = ((uint32_t)addr[0] & 0x000000FF);
      taddr = (taddr << 8) + ((uint32_t)addr[1] & 0x000000FF);
      taddr = (taddr << 8) + ((uint32_t)addr[2] & 0x000000FF);
      taddr = (taddr << 8) + ((uint32_t)addr[3] & 0x000000FF);
      if( taddr == 0xFFFFFFFF || taddr == 0) return SOCKERR_IPINVALID;
   }
   //

	if(port == 0) return SOCKERR_PORTZERO;
	setSn_DIPR(sn,addr);
	setSn_DPORT(sn,port);
	setSn_CR(sn,Sn_CR_CONNECT);
   while(getSn_CR(sn));
   if(sock_io_mode & (1<<sn)) return SOCK_BUSY;
   while(getSn_SR(sn) != SOCK_ESTABLISHED)
   {
		if (getSn_IR(sn) & Sn_IR_TIMEOUT)
		{
			setSn_IR(sn, Sn_IR_TIMEOUT);
            return SOCKERR_TIMEOUT;
		}

		if (getSn_SR(sn) == SOCK_CLOSED)
		{
			return SOCKERR_SOCKCLOSED;
		}
	}

   return SOCK_OK;
}

int8_t disconnect(uint8_t sn)
{
   CHECK_SOCKNUM();
   CHECK_SOCKMODE(Sn_MR_TCP);
	setSn_CR(sn,Sn_CR_DISCON);
	/* wait to process the command... */
	while(getSn_CR(sn));
	sock_is_sending &= ~(1<<sn);
   if(sock_io_mode & (1<<sn)) return SOCK_BUSY;
	while(getSn_SR(sn) != SOCK_CLOSED)
	{
	   if(getSn_IR(sn) & Sn_IR_TIMEOUT)
	   {
	      close(sn);
	      return SOCKERR_TIMEOUT;
	   }
	}
	return SOCK_OK;
}

int32_t send(uint8_t sn, uint8_t * buf, uint16_t len)
{
   uint8_t tmp=0;
   uint16_t freesize=0;

   CHECK_SOCKNUM();
   CHECK_SOCKMODE(Sn_MR_TCP);
   CHECK_SOCKDATA();
   tmp = getSn_SR(sn);
   if(tmp != SOCK_ESTABLISHED && tmp != SOCK_CLOSE_WAIT) return SOCKERR_SOCKSTATUS;
   if( sock_is_sending & (1<<sn) )
   {
      tmp = getSn_IR(sn);
      if(tmp & Sn_IR_SENDOK)
      {
         setSn_IR(sn, Sn_IR_SENDOK);
         //M20150401 : Typing Error
         sock_is_sending &= ~(1<<sn);
      }
      else if(tmp & Sn_IR_TIMEOUT)
      {
         close(sn);
         return SOCKERR_TIMEOUT;
      }
      else return SOCK_BUSY;
   }
   freesize = getSn_TxMAX(sn);
   if (len > freesize) len = freesize; // check size not to exceed MAX size.
   while(1)
   {
      freesize = getSn_TX_FSR(sn);
      tmp = getSn_SR(sn);
      if ((tmp != SOCK_ESTABLISHED) && (tmp != SOCK_CLOSE_WAIT))
      {
         close(sn);
         return SOCKERR_SOCKSTATUS;
      }
      if( (sock_io_mode & (1<<sn)) && (len > freesize) ) return SOCK_BUSY;
      if(len <= freesize) break;
   }
   wiz_send_data(sn, buf, len);

   setSn_CR(sn,Sn_CR_SEND);
   /* wait to process the command... */
   while(getSn_CR(sn));
   sock_is_sending |= (1 << sn);
   //M20150409 : Explicit Type Casting
   //return len;
   return (int32_t)len;
}


int32_t recv(uint8_t sn, uint8_t * buf, uint16_t len)
{
   uint8_t  tmp = 0;
   uint16_t recvsize = 0;
//A20150601 : For integarating with W5300
//
   CHECK_SOCKNUM();
   CHECK_SOCKMODE(Sn_MR_TCP);
   CHECK_SOCKDATA();

   recvsize = getSn_RxMAX(sn);
   if(recvsize < len) len = recvsize;

//A20150601 : For Integrating with W5300
//
      while(1)
      {
         recvsize = getSn_RX_RSR(sn);
         tmp = getSn_SR(sn);
         if (tmp != SOCK_ESTABLISHED)
         {
            if(tmp == SOCK_CLOSE_WAIT)
            {
               if(recvsize != 0) break;
               else if(getSn_TX_FSR(sn) == getSn_TxMAX(sn))
               {
                  close(sn);
                  return SOCKERR_SOCKSTATUS;
               }
            }
            else
            {
               close(sn);
               return SOCKERR_SOCKSTATUS;
            }
         }
         if((sock_io_mode & (1<<sn)) && (recvsize == 0)) return SOCK_BUSY;
         if(recvsize != 0) break;
      };

//A20150601 : For integrating with W5300

   if(recvsize < len) len = recvsize;
   wiz_recv_data(sn, buf, len);
   setSn_CR(sn,Sn_CR_RECV);
   while(getSn_CR(sn));


   //M20150409 : Explicit Type Casting
   //return len;
   return (int32_t)len;
}

int32_t sendto(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port)
{
   uint8_t tmp = 0;
   uint16_t freesize = 0;
   uint32_t taddr;

   CHECK_SOCKNUM();
   switch(getSn_MR(sn) & 0x0F)
   {
      case Sn_MR_UDP:
      case Sn_MR_MACRAW:
//         break;
//   #if ( _WIZCHIP_ < 5200 )
      case Sn_MR_IPRAW:
         break;
//   #endif
      default:
         return SOCKERR_SOCKMODE;
   }
   CHECK_SOCKDATA();
   //M20140501 : For avoiding fatal error on memory align mismatched
   //if(*((uint32_t*)addr) == 0) return SOCKERR_IPINVALID;
   //{
      //uint32_t taddr;
      taddr = ((uint32_t)addr[0]) & 0x000000FF;
      taddr = (taddr << 8) + ((uint32_t)addr[1] & 0x000000FF);
      taddr = (taddr << 8) + ((uint32_t)addr[2] & 0x000000FF);
      taddr = (taddr << 8) + ((uint32_t)addr[3] & 0x000000FF);
   //}
   //
   //if(*((uint32_t*)addr) == 0) return SOCKERR_IPINVALID;
   if((taddr == 0) && ((getSn_MR(sn)&Sn_MR_MACRAW) != Sn_MR_MACRAW)) return SOCKERR_IPINVALID;
   if((port  == 0) && ((getSn_MR(sn)&Sn_MR_MACRAW) != Sn_MR_MACRAW)) return SOCKERR_PORTZERO;
   tmp = getSn_SR(sn);
//#if ( _WIZCHIP_ < 5200 )
   if((tmp != SOCK_MACRAW) && (tmp != SOCK_UDP) && (tmp != SOCK_IPRAW)) return SOCKERR_SOCKSTATUS;
//#else
//   if(tmp != SOCK_MACRAW && tmp != SOCK_UDP) return SOCKERR_SOCKSTATUS;
//#endif

   setSn_DIPR(sn,addr);
   setSn_DPORT(sn,port);
   freesize = getSn_TxMAX(sn);
   if (len > freesize) len = freesize; // check size not to exceed MAX size.
   while(1)
   {
      freesize = getSn_TX_FSR(sn);
      if(getSn_SR(sn) == SOCK_CLOSED) return SOCKERR_SOCKCLOSED;
      if( (sock_io_mode & (1<<sn)) && (len > freesize) ) return SOCK_BUSY;
      if(len <= freesize) break;
   };
	wiz_send_data(sn, buf, len);

//
	setSn_CR(sn,Sn_CR_SEND);
	/* wait to process the command... */
	while(getSn_CR(sn));
   while(1)
   {
      tmp = getSn_IR(sn);
      if(tmp & Sn_IR_SENDOK)
      {
         setSn_IR(sn, Sn_IR_SENDOK);
         break;
      }
      //M:20131104
      //else if(tmp & Sn_IR_TIMEOUT) return SOCKERR_TIMEOUT;
      else if(tmp & Sn_IR_TIMEOUT)
      {
         setSn_IR(sn, Sn_IR_TIMEOUT);
         //M20150409 : Fixed the lost of sign bits by type casting.
         //len = (uint16_t)SOCKERR_TIMEOUT;
         //break;
         return SOCKERR_TIMEOUT;
      }
      ////////////
   }
   //M20150409 : Explicit Type Casting
   //return len;
   return (int32_t)len;
}



int32_t recvfrom(uint8_t sn, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t *port)
{
//M20150601 : For W5300
   uint8_t  mr;

//
   uint8_t  head[8];
	uint16_t pack_len=0;

   CHECK_SOCKNUM();
   //CHECK_SOCKMODE(Sn_MR_UDP);
//A20150601

   switch((mr=getSn_MR(sn)) & 0x0F)
   {
      case Sn_MR_UDP:
	  case Sn_MR_IPRAW:
      case Sn_MR_MACRAW:
         break;

      default:
         return SOCKERR_SOCKMODE;
   }
   CHECK_SOCKDATA();
   if(sock_remained_size[sn] == 0)
   {
      while(1)
      {
         pack_len = getSn_RX_RSR(sn);
         if(getSn_SR(sn) == SOCK_CLOSED) return SOCKERR_SOCKCLOSED;
         if( (sock_io_mode & (1<<sn)) && (pack_len == 0) ) return SOCK_BUSY;
         if(pack_len != 0) break;
      };
   }
//D20150601 : Move it to bottom
// sock_pack_info[sn] = PACK_COMPLETED;
	switch (mr & 0x07)
	{
	   case Sn_MR_UDP :
	      if(sock_remained_size[sn] == 0)
	      {
   			wiz_recv_data(sn, head, 8);
   			setSn_CR(sn,Sn_CR_RECV);
   			while(getSn_CR(sn));
   			// read peer's IP address, port number & packet length
   	   //A20150601 : For W5300

               addr[0] = head[0];
      			addr[1] = head[1];
      			addr[2] = head[2];
      			addr[3] = head[3];
      			*port = head[4];
      			*port = (*port << 8) + head[5];
      			sock_remained_size[sn] = head[6];
      			sock_remained_size[sn] = (sock_remained_size[sn] << 8) + head[7];

   			sock_pack_info[sn] = PACK_FIRST;
   	   }
			if(len < sock_remained_size[sn]) pack_len = len;
			else pack_len = sock_remained_size[sn];
			//A20150601 : For W5300
			len = pack_len;

			//
			// Need to packet length check (default 1472)
			//
   		wiz_recv_data(sn, buf, pack_len); // data copy.
			break;
	   case Sn_MR_MACRAW :
	      if(sock_remained_size[sn] == 0)
	      {
   			wiz_recv_data(sn, head, 2);
   			setSn_CR(sn,Sn_CR_RECV);
   			while(getSn_CR(sn));
   			// read peer's IP address, port number & packet length
    			sock_remained_size[sn] = head[0];
   			sock_remained_size[sn] = (sock_remained_size[sn] <<8) + head[1] -2;

   			if(sock_remained_size[sn] > 1514)
   			{
   			   close(sn);
   			   return SOCKFATAL_PACKLEN;
   			}
   			sock_pack_info[sn] = PACK_FIRST;
   	   }
			if(len < sock_remained_size[sn]) pack_len = len;
			else pack_len = sock_remained_size[sn];
			wiz_recv_data(sn,buf,pack_len);
		   break;

		case Sn_MR_IPRAW:
		   if(sock_remained_size[sn] == 0)
		   {
   			wiz_recv_data(sn, head, 6);
   			setSn_CR(sn,Sn_CR_RECV);
   			while(getSn_CR(sn));
   			addr[0] = head[0];
   			addr[1] = head[1];
   			addr[2] = head[2];
   			addr[3] = head[3];
   			sock_remained_size[sn] = head[4];
   			//M20150401 : For Typing Error
   			//sock_remaiend_size[sn] = (sock_remained_size[sn] << 8) + head[5];
   			sock_remained_size[sn] = (sock_remained_size[sn] << 8) + head[5];
   			sock_pack_info[sn] = PACK_FIRST;
         }
			//
			// Need to packet length check
			//
			if(len < sock_remained_size[sn]) pack_len = len;
			else pack_len = sock_remained_size[sn];
   		wiz_recv_data(sn, buf, pack_len); // data copy.
			break;
   //#endif
      default:
         wiz_recv_ignore(sn, pack_len); // data copy.
         sock_remained_size[sn] = pack_len;
         break;
   }
	setSn_CR(sn,Sn_CR_RECV);
	/* wait to process the command... */
	while(getSn_CR(sn)) ;
	sock_remained_size[sn] -= pack_len;
	//M20150601 :
	//if(sock_remained_size[sn] != 0) sock_pack_info[sn] |= 0x01;
	if(sock_remained_size[sn] != 0)
	{
	   sock_pack_info[sn] |= PACK_REMAINED;

	}
	else sock_pack_info[sn] = PACK_COMPLETED;

   //
   //M20150409 : Explicit Type Casting
   //return pack_len;
   return (int32_t)pack_len;
}


int8_t  ctlsocket(uint8_t sn, ctlsock_type cstype, void* arg)
{
   uint8_t tmp = 0;
   CHECK_SOCKNUM();
   switch(cstype)
   {
      case CS_SET_IOMODE:
         tmp = *((uint8_t*)arg);
         if(tmp == SOCK_IO_NONBLOCK)  sock_io_mode |= (1<<sn);
         else if(tmp == SOCK_IO_BLOCK) sock_io_mode &= ~(1<<sn);
         else return SOCKERR_ARG;
         break;
      case CS_GET_IOMODE:
         //M20140501 : implict type casting -> explict type casting
         //*((uint8_t*)arg) = (sock_io_mode >> sn) & 0x0001;
         *((uint8_t*)arg) = (uint8_t)((sock_io_mode >> sn) & 0x0001);
         //
         break;
      case CS_GET_MAXTXBUF:
         *((uint16_t*)arg) = getSn_TxMAX(sn);
         break;
      case CS_GET_MAXRXBUF:
         *((uint16_t*)arg) = getSn_RxMAX(sn);
         break;
      case CS_CLR_INTERRUPT:
         if( (*(uint8_t*)arg) > SIK_ALL) return SOCKERR_ARG;
         setSn_IR(sn,*(uint8_t*)arg);
         break;
      case CS_GET_INTERRUPT:
         *((uint8_t*)arg) = getSn_IR(sn);
         break;

      case CS_SET_INTMASK:
         if( (*(uint8_t*)arg) > SIK_ALL) return SOCKERR_ARG;
         setSn_IMR(sn,*(uint8_t*)arg);
         break;
      case CS_GET_INTMASK:
         *((uint8_t*)arg) = getSn_IMR(sn);
         break;

      default:
         return SOCKERR_ARG;
   }
   return SOCK_OK;
}

int8_t  setsockopt(uint8_t sn, sockopt_type sotype, void* arg)
{
 // M20131220 : Remove warning
 //uint8_t tmp;
   CHECK_SOCKNUM();
   switch(sotype)
   {
      case SO_TTL:
         setSn_TTL(sn,*(uint8_t*)arg);
         break;
      case SO_TOS:
         setSn_TOS(sn,*(uint8_t*)arg);
         break;
      case SO_MSS:
         setSn_MSSR(sn,*(uint16_t*)arg);
         break;
      case SO_DESTIP:
         setSn_DIPR(sn, (uint8_t*)arg);
         break;
      case SO_DESTPORT:
         setSn_DPORT(sn, *(uint16_t*)arg);
         break;

      case SO_KEEPALIVESEND:
         CHECK_SOCKMODE(Sn_MR_TCP);
            if(getSn_KPALVTR(sn) != 0) return SOCKERR_SOCKOPT;
            setSn_CR(sn,Sn_CR_SEND_KEEP);
            while(getSn_CR(sn) != 0)
            {
               // M20131220
         		//if ((tmp = getSn_IR(sn)) & Sn_IR_TIMEOUT)
               if (getSn_IR(sn) & Sn_IR_TIMEOUT)
         		{
         			setSn_IR(sn, Sn_IR_TIMEOUT);
                  return SOCKERR_TIMEOUT;
         		}
            }
         break;
      case SO_KEEPALIVEAUTO:
         CHECK_SOCKMODE(Sn_MR_TCP);
         setSn_KPALVTR(sn,*(uint8_t*)arg);
         break;
      default:
         return SOCKERR_ARG;
   }
   return SOCK_OK;
}

int8_t  getsockopt(uint8_t sn, sockopt_type sotype, void* arg)
{
   CHECK_SOCKNUM();
   switch(sotype)
   {
      case SO_FLAG:
         *(uint8_t*)arg = getSn_MR(sn) & 0xF0;
         break;
      case SO_TTL:
         *(uint8_t*) arg = getSn_TTL(sn);
         break;
      case SO_TOS:
         *(uint8_t*) arg = getSn_TOS(sn);
         break;
      case SO_MSS:
         *(uint16_t*) arg = getSn_MSSR(sn);
         break;
      case SO_DESTIP:
         getSn_DIPR(sn, (uint8_t*)arg);
         break;
      case SO_DESTPORT:
         *(uint16_t*) arg = getSn_DPORT(sn);
         break;
      case SO_KEEPALIVEAUTO:
         CHECK_SOCKMODE(Sn_MR_TCP);
         *(uint16_t*) arg = getSn_KPALVTR(sn);
         break;
      case SO_SENDBUF:
         *(uint16_t*) arg = getSn_TX_FSR(sn);
         break;
      case SO_RECVBUF:
         *(uint16_t*) arg = getSn_RX_RSR(sn);
         break;
      case SO_STATUS:
         *(uint8_t*) arg = getSn_SR(sn);
         break;
      case SO_REMAINSIZE:
         if(getSn_MR(sn) & Sn_MR_TCP)
            *(uint16_t*)arg = getSn_RX_RSR(sn);
         else
            *(uint16_t*)arg = sock_remained_size[sn];
         break;
      case SO_PACKINFO:
         //CHECK_SOCKMODE(Sn_MR_TCP);
         if((getSn_MR(sn) == Sn_MR_TCP))
             return SOCKERR_SOCKMODE;
         *(uint8_t*)arg = sock_pack_info[sn];
         break;
      default:
         return SOCKERR_SOCKOPT;
   }
   return SOCK_OK;
}
