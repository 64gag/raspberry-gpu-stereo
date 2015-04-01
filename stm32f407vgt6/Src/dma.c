#include "stm32f4xx_hal.h"
#include "dma.h"
#include "debflags.h"

extern uint8_t dcmi_buff[];

void DcmiDma_Init(void)
{
  /* DMA and DCMI interrupts */
  DMA2->LIFCR = 0xffffffff; /* Make sure all the IF are cleared */
  DCMI->ICR = 0x1f;

  #if DEB_INT
  HAL_NVIC_SetPriority(DCMI_IRQn, 5, 0); /* Enable interrupts */
  HAL_NVIC_EnableIRQ(DCMI_IRQn);
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 5, 0);   
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
  #endif
  
  __DMA2_CLK_ENABLE();
  __DCMI_CLK_ENABLE();

  /* DMA2 configuration and intialization */
  DMA2_Stream1->CR &= 0xf0100000;
  while(DMA2_Stream1->CR & 1){}         /* Wait until DMA is disabled */  
  DMA2_Stream1->NDTR = (uint32_t)(IMG_W*IMG_H/2);/* Number of data transfers */
  DMA2_Stream1->PAR = (uint32_t)(&(DCMI->DR));
  DMA2_Stream1->M0AR = (uint32_t)(&(dcmi_buff[0])); /* DMA address registers */
  DMA2_Stream1->M1AR = (uint32_t)(&(dcmi_buff[0]));

  DMA2_Stream1->CR |= /* Channel [0,7] */             1<<25| \
              /* Mburst */                            0<<23| \
              /* Pburst */                            0<<21| \
              /* Current target */                    0<<19| \
              /* Double buffer */                     0<<18| \
              /* Priority [0,3] */                    2<<16| \
              /* Per inc offset (0= %Psize) */        1<<15| \
              /* Mem size (0b10=32bits) */            2<<13| \
              /* Periph size (0b10=32bits) */         2<<11| \
              /* Memory increment */                  1<<10| \
              /* Periph increment */                  0<< 9| \
              /* Circular mode */                     1<< 8| \
              /* Dir (0b00 == P2M) */                 0<< 6| \
              /* Periph flow controller */            0<< 5| \
              /* Transfer complete IE */              0<< 4| \
              /* Half transfer IE */                  0<< 3| \
              /* Transfer error IE */                 0<< 2| \
              /* Direct mode error IE */              0<< 1| \
              /* Stream EN/Ready */                   0<< 0;  
  
  DMA2_Stream1->FCR |= /* FIFO error IE */            0<< 7| \
                       /* Direct mode disable */      1<< 2| \
                       /* FIFO thres (1/4)*(X+1)*/    3<< 0;

  DMA2_Stream1->CR |= 1;                     /* Enable */
  while(!(DMA2_Stream1->CR & 1));            /* Wait until DMA is enabled */
 
  /* DCMI configuration and intialization */
  DCMI->CR &= 0xffffb000;               /* Clear all non-reserved bits */        
  while(DCMI->CR & 0x00004000);         /* Wait until peripheral is disabled */       
  DCMI->CR |= /* Extended data 00 = 8 bit*/         0<<10| \
              /* Frame capture 00 = all */          0<< 9| \
              /* VSPOL*/                            1<< 7| \
              /* HSPOL */                           0<< 6| \
              /* PCK polarity */                    1<< 5| \
              /* Embedded syncro */                 0<< 4| \
              /* JPEG */                            0<< 3| \
              /* Crop*/                             0<< 2| \
              /* Capture mode, 0 = continuous */    0<< 1| \
              /* Capture enable */                  1<< 0;
  
  DCMI->IER = /* LINE_IE */                         0<< 4| \
              /* VSYNC_IE */                        0<< 3| \
              /* ERR_IE */                          0<< 2| \
              /* OVR_IE */                          0<< 1| \
              /* FRAME_IE */                        0<< 0;

  DCMI->CR |= 0x00004000;               /* Enable DCMI */
  while(!(DCMI->CR & 0x00004000));      /* Wait until enabled */
}

void DcmiDma_Pause(void)
{
  DCMI->CR &= 0xffffbfff;               /* Disable DCMI */
  while(DCMI->CR & 0x00004000);         /* Wait until peripheral is disabled */

  DMA2_Stream1->CR &= 0xfffffffe;       /* Disable DMA */        
  while(DMA2_Stream1->CR & 1);         /* Wait until DMA is disabled */
}

void DcmiDma_Resume(void)
{
  /* DMA and DCMI interrupts */
  DMA2->LIFCR = 0xffffffff; /* Make sure all the IF are cleared */
  DMA2->HIFCR = 0xffffffff; 
  DCMI->ICR = 0x1f;

  DMA2_Stream1->CR &= 0xfffffffe;
  while(DMA2_Stream1->CR & 1);         /* Wait until DMA is disabled */  
  DMA2_Stream1->NDTR = (uint32_t)(IMG_W*IMG_H/2);   /* Number of data transfers */
  DMA2_Stream1->PAR = (uint32_t)(&(DCMI->DR));
  DMA2_Stream1->M0AR = (uint32_t)(&(dcmi_buff[0])); /* DMA address registers */
  DMA2_Stream1->M1AR = (uint32_t)(&(dcmi_buff[0]));
  DMA2_Stream1->CR |= 1;                     /* Enable */
  while(!(DMA2_Stream1->CR & 1));            /* Wait until DMA is enabled */

  DCMI->CR |= 0x00004000;               /* Enable DCMI */
  while(!(DCMI->CR & 0x00004000));      /* Wait until enabled */
}


void SpiDma_Init(void)
{
  /* DMA interrupt */
  DMA2->HIFCR = 0xffffffff; /* Make sure all the IF are cleared */

  HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 5, 0);   
  HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);

  __DMA2_CLK_ENABLE();
  __SPI1_CLK_ENABLE();

  /* DMA2 configuration and intialization */
  DMA2_Stream5->CR &= 0xf0100000;
  while(DMA2_Stream5->CR & 1){}         /* Wait until DMA is disabled */  
  DMA2_Stream5->NDTR = COMPLETE_NDTR;   /* Number of data transfers */
  DMA2_Stream5->PAR = (uint32_t)(&(SPI1->DR));
  DMA2_Stream5->M0AR = (uint32_t)(&(dcmi_buff[0])); /* DMA address registers */
  DMA2_Stream5->M1AR = (uint32_t)(&(dcmi_buff[0]));

  DMA2_Stream5->CR |= /* Channel [0,7] */             3<<25| \
              /* Mburst */                            1<<23| \
              /* Pburst */                            1<<21| \
              /* Current target */                    0<<19| \
              /* Double buffer */                     1<<18| \
              /* Priority [0,3] */                    3<<16| \
              /* Per inc offset (0= %Psize) */        0<<15| \
              /* Mem size */                          1<<13| \
              /* Periph size (0b00=8bits) */          1<<11| \
              /* Memory increment */                  1<<10| \
              /* Periph increment */                  0<< 9| \
              /* Circular mode */                     1<< 8| \
              /* Dir (0b01 == M2P) */                 1<< 6| \
              /* Periph flow controller */            0<< 5| \
              /* Transfer complete IE */              0<< 4| \
              /* Half transfer IE */                  1<< 3| \
              /* Transfer error IE */                 0<< 2| \
              /* Direct mode error IE */              0<< 1| \
              /* Stream EN/Ready */                   0<< 0;  
  
  DMA2_Stream5->FCR |= /* FIFO error IE */            0<< 7| \
                       /* Direct mode disable */      0<< 2| \
                       /* FIFO thres (1/4)*(X+1)*/    0<< 0;

  DMA2_Stream5->CR |= 1;                     /* Enable */
  while(!(DMA2_Stream5->CR & 1));            /* Wait until DMA is enabled */
 
  /* SPI configuration and intialization */
  SPI1->CR1 &= 0x0000;               /* Clear all non-reserved bits */        
  SPI1->CR2 &= 0xff08;
  
  while(SPI1->CR1 & 0x0040);         /* Wait until peripheral is disabled */       
  SPI1->CR1 |= /* BIDIMODE */                     0<<15| \
               /* BIDIOE */                       0<<14| \
               /* CRCEN */                        0<<13| \
               /* CRCNEXT */                      0<<12| \
               /* Data frame 0=8b, 1=16b */       0<<11| \
               /* RX only */                      0<<10| \
               /* Slave management */             0<< 9| \
               /* Slave select */                 0<< 8| \
               /* LSBfirst */                     0<< 7| \
               /* SPI enable */                   0<< 6| \
               /* Baud rate control */            0<< 3| \
               /* Master selection */             0<< 2| \
               /* Clock polarity when idle */     1<< 1| \
               /* Clock phase */                  0<< 0;
  
  SPI1->CR2 |= /* TXEIE */                        0<< 7| \
               /* RXNEIE */                       0<< 6| \
               /* Error interrupt enable */       0<< 5| \
               /* Frame format */                 0<< 4| \
               /* SS output enable */             0<< 2| \
               /* TXDMAEN */                      1<< 1| \
               /* RXDMAEN */                      0<< 0;
}

void SpiDma_Reset(void)
{
  /* DMA interrupts clear */
  DMA2->HIFCR = 0xffffffff; /* Make sure all the IF are cleared */

  /* Disable DMA stream and peripheral*/
  Spi_Disable();
  DMA2_Stream5->CR &= 0xfffffffe;
  while(DMA2_Stream5->CR & 1){}         /* Wait until DMA is disabled */

  /* Reconfigure important registers */
  DMA2_Stream5->CR &= 0xfff7ffff;       /* Make sure CT = 0 */
  DMA2_Stream5->NDTR = COMPLETE_NDTR;          /* Number of data transfers */
  DMA2_Stream5->PAR = (uint32_t)(&(SPI1->DR));
  DMA2_Stream5->M0AR = (uint32_t)(&(dcmi_buff[0])); /* DMA address registers */
  DMA2_Stream5->M1AR = (uint32_t)(&(dcmi_buff[0]));
  
  /* Re-enable stream */
  DMA2_Stream5->CR |= 1;                /* Enable */
  while(!(DMA2_Stream5->CR & 1));       /* Wait until DMA is enabled */
}

void Spi_Enable(void){
  SPI1->CR1 |= 0x0040;                  /* Enable SPI1 */
  while(!(SPI1->CR1 & 0x0040));         /* Wait until peripheral is enabled */
}

void Spi_Disable(void){
  SPI1->CR1 &= 0xffbf;
  while(SPI1->CR1 & 0x0040);            /* Wait until peripheral is disabled */   
}
