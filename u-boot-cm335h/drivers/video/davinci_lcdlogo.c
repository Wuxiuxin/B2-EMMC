#include <common.h>
#include <asm/io.h>
//#include <lcd.h>
//#include <nand.h>
#include <mmc.h>

DECLARE_GLOBAL_DATA_PTR;
unsigned short image1[384016];
unsigned short array_head[16]={
0x4000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u,0x0000u,
0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u
};

unsigned short *ptr_lcd =(unsigned short *)(0x82000000);


//#define _LCD_43
#define _LCD_7
//#define _LCD_28

#define LCDC_RASTER_CTRL_RASTER_EN             (0x00000001u)
#define LCDC_LCD_CTRL_MODESEL                  (0x00000001u)
#define LCDC_LCD_CTRL_CLKDIV_SHIFT             (0x00000008u)
#define RASTER_DOUBLE_FRAME_BUFFER             (0x00000001u)
#define LCDC_LCDDMA_CTRL_BURST_SIZE_SHIFT      (0x00000004u)
#define RASTER_BURST_SIZE_16                   (4 <<  LCDC_LCDDMA_CTRL_BURST_SIZE_SHIFT)
#define LCDC_LCDDMA_CTRL_TH_FIFO_READY_SHIFT   (0x00000008u)
#define RASTER_FIFO_THRESHOLD_8                (0 << LCDC_LCDDMA_CTRL_TH_FIFO_READY_SHIFT) 
#define RASTER_BIG_ENDIAN_DISABLE              0
#define LCDC_RASTER_CTRL_TFT_STN_SHIFT         (0x00000007u)
#define RASTER_DISPLAY_MODE_TFT                (1 << LCDC_RASTER_CTRL_TFT_STN_SHIFT) 
#define RASTER_PALETTE_DATA                    0
#define RASTER_COLOR                           0
#define RASTER_RIGHT_ALIGNED                   0
#define LCDC_RASTER_CTRL_TFT_ALT_MAP           (0x00800000u)
#define LCDC_RASTER_CTRL_RD_ORDER              (0x00000100u)
#define LCDC_RASTER_CTRL_NIB_MODE              (0x00400000u)
#define LCDC_RASTER_TIMING_2_ACB_I_SHIFT       (0x00000010u)
#define LCDC_RASTER_TIMING_2_ACB_SHIFT         (0x00000008u) 

#define LCDC_RASTER_TIMING_0_HBP_SHIFT   (0x00000018u)
#define LCDC_RASTER_TIMING_0_HFP_SHIFT   (0x00000010u)
#define LCDC_RASTER_TIMING_0_HSW_SHIFT   (0x0000000Au)
#define LCDC_RASTER_TIMING_0_PPL_SHIFT   (0x00000004u)
#define LCDC_RASTER_TIMING_0_PPLMSB_SHIFT   (0x00000003u)

#define LCDC_RASTER_TIMING_1_LPP_SHIFT   (0x00000000u)
#define LCDC_RASTER_TIMING_1_VBP_SHIFT   (0x00000018u)
#define LCDC_RASTER_TIMING_1_VFP_SHIFT   (0x00000010u)
#define LCDC_RASTER_TIMING_1_VSW_SHIFT   (0x0000000Au)

#define LCDC_RASTER_CTRL_FIFO_DMA_DELAY   (0x000FF000u)
#define LCDC_RASTER_CTRL_FIFO_DMA_DELAY_SHIFT   (0x0000000Cu)


#define LCDC_RASTER_TIMING_2_IVS               (0x00100000u)
#define RASTER_FRAME_CLOCK_LOW                 LCDC_RASTER_TIMING_2_IVS

#define LCDC_RASTER_TIMING_2_IHS               (0x00200000u)
#define RASTER_LINE_CLOCK_LOW                  LCDC_RASTER_TIMING_2_IHS

#define RASTER_PIXEL_CLOCK_LOW                 (0x00400000u)

#define RASTER_SYNC_EDGE_RISING                0

#define RASTER_SYNC_CTRL_ACTIVE                (0x02000000u)

#define RASTER_AC_BIAS_HIGH                    0
#define RASTER_PIXEL_CLOCK_HIGH                0

#define SOC_LCDC_0_REGS                        (0x4830E000)
 
#define LCDC_RASTER_CTRL                       (0x28)
#define LCDC_LCD_CTRL                          (0x4)
#define LCDC_LCDDMA_CTRL                       (0x40)
#define LCDC_RASTER_TIMING_2                   (0x34)
#define LCDC_RASTER_TIMING_0                   (0x2C)
#define LCDC_RASTER_TIMING_1                   (0x30)

#define LCDC_LCDDMA_FB0_BASE                   (0x44)
#define LCDC_LCDDMA_FB0_CEILING                (0x48)
#define LCDC_LCDDMA_FB1_BASE                   (0x4C)
#define LCDC_LCDDMA_FB1_CEILING                (0x50)

#define LCDC_CLKC_ENABLE                       (0x6c)

#define RASTER_REV_AM335X                    2u
#define RASTER_REV_AM1808                    1u

#define REG(addr) *((volatile unsigned int *)(addr))

static void RasterEnableClock(unsigned int baseaddr)
{
	REG(baseaddr + LCDC_CLKC_ENABLE ) = 0x7;
}

static void RasterDisable(unsigned int baseAddr)
{
    REG(baseAddr + LCDC_RASTER_CTRL) &= ~LCDC_RASTER_CTRL_RASTER_EN;
}

static void RasterClkConfig(unsigned int baseAddr, unsigned int pClk,
                     unsigned int moduleClk)
{
    unsigned int clkDiv;

    clkDiv = moduleClk / pClk ;

    REG(baseAddr + LCDC_LCD_CTRL) = LCDC_LCD_CTRL_MODESEL;

    REG(baseAddr + LCDC_LCD_CTRL) |= (clkDiv <<  LCDC_LCD_CTRL_CLKDIV_SHIFT);
}
static void RasterDMAConfig(unsigned int baseAddr, unsigned int frmMode,
                     unsigned int bustSz, unsigned int fifoTh,
                     unsigned int endian)
{
    REG(baseAddr + LCDC_LCDDMA_CTRL) = frmMode | bustSz | fifoTh | endian;
}
static void RasterModeConfig(unsigned int baseAddr, unsigned int displayMode,
                      unsigned int paletteMode, unsigned int displayType,
                      unsigned flag)
{
    /* Configures raster to TFT or STN Mode */
    REG(baseAddr + LCDC_RASTER_CTRL) = displayMode | paletteMode | displayType;

         if(flag == RASTER_RIGHT_ALIGNED)
         {
              /* Output pixel data for 1,2,4 and 8 bpp is converted to 565 format */
              REG(baseAddr + LCDC_RASTER_CTRL) &= ~(LCDC_RASTER_CTRL_TFT_ALT_MAP);
         }
         else
         {
              /* Output pixel data for 1,2,4 and 8 bpp will be right aligned */
             REG(baseAddr + LCDC_RASTER_CTRL) |= LCDC_RASTER_CTRL_TFT_ALT_MAP;
         }
}
static void RasterLSBDataOrderSelect(unsigned int baseAddr)
{
    REG(baseAddr + LCDC_RASTER_CTRL) &= ~LCDC_RASTER_CTRL_RD_ORDER;
}
static void RasterNibbleModeDisable(unsigned int baseAddr)
{
    REG(baseAddr + LCDC_RASTER_CTRL) &= ~LCDC_RASTER_CTRL_NIB_MODE;
}
static void RasterTiming2Configure(unsigned int baseAddr, unsigned int flag,
                            unsigned int acb_i, unsigned int acb)
{
    REG(baseAddr + LCDC_RASTER_TIMING_2) |= flag;

    REG(baseAddr + LCDC_RASTER_TIMING_2) |= (acb_i <<                    \
                                              LCDC_RASTER_TIMING_2_ACB_I_SHIFT);

    REG(baseAddr + LCDC_RASTER_TIMING_2) |= (acb <<                      \
                                             LCDC_RASTER_TIMING_2_ACB_SHIFT);
}
static unsigned int LCDVersionGet(void)
{
    return 1;
}
static void RasterHparamConfig(unsigned int baseAddr, unsigned int numOfppl,
                        unsigned int hsw, unsigned int hfp,
                        unsigned hbp)
{
    unsigned int ppl;
    unsigned int version;

    #ifdef _LCD_7
	REG(baseAddr + LCDC_RASTER_TIMING_2) |= (1<<22);
    #endif

    version = LCDVersionGet();

    if(RASTER_REV_AM1808 == version)
    {

         ppl = (numOfppl / 16) - 1;

         REG(baseAddr + LCDC_RASTER_TIMING_0) =  (ppl <<
                                                LCDC_RASTER_TIMING_0_PPL_SHIFT);
    }
    else
    {
         ;/* Do nothing */
    }

    REG(baseAddr + LCDC_RASTER_TIMING_0) |= ((hsw - 1) <<
                                               LCDC_RASTER_TIMING_0_HSW_SHIFT);

    REG(baseAddr + LCDC_RASTER_TIMING_0) |= ((hfp - 1) <<
                                               LCDC_RASTER_TIMING_0_HFP_SHIFT);

    REG(baseAddr + LCDC_RASTER_TIMING_0) |= ((hbp - 1) <<
                                               LCDC_RASTER_TIMING_0_HBP_SHIFT);
}
static void RasterVparamConfig(unsigned int baseAddr, unsigned int lpp,
                        unsigned int vsw, unsigned int vfp,
                        unsigned vbp)
{
    unsigned int version;

    version = LCDVersionGet();

    if(RASTER_REV_AM335X == version)
    {
    }
    else if(RASTER_REV_AM1808 == version)
    {

         REG(baseAddr + LCDC_RASTER_TIMING_1) =  ((lpp - 1) <<
                                              LCDC_RASTER_TIMING_1_LPP_SHIFT);
    }
    else
    {
         ;/* Do nothing */
    }

    REG(baseAddr + LCDC_RASTER_TIMING_1) |= ((vsw - 1) <<
                                               LCDC_RASTER_TIMING_1_VSW_SHIFT);

    REG(baseAddr + LCDC_RASTER_TIMING_1) |= (vfp <<
                                               LCDC_RASTER_TIMING_1_VFP_SHIFT);

    REG(baseAddr + LCDC_RASTER_TIMING_1) |= (vbp <<
                                               LCDC_RASTER_TIMING_1_VBP_SHIFT);
}

static void RasterFIFODMADelayConfig(unsigned int baseAddr, unsigned int delay)
{

    REG(baseAddr + LCDC_RASTER_CTRL) &= ~LCDC_RASTER_CTRL_FIFO_DMA_DELAY;
    REG(baseAddr + LCDC_RASTER_CTRL) |= (delay <<                  \
                                          LCDC_RASTER_CTRL_FIFO_DMA_DELAY_SHIFT);
}
static void RasterEnable(unsigned int baseAddr)
{
    REG(baseAddr + LCDC_RASTER_CTRL) |= LCDC_RASTER_CTRL_RASTER_EN;
}
static void RasterDMAFBConfig(unsigned int baseAddr, unsigned int base,
                       unsigned int  ceiling, unsigned int flag)
{
    if(flag == 0)
    {
         REG(baseAddr + LCDC_LCDDMA_FB0_BASE) =  base;
         REG(baseAddr + LCDC_LCDDMA_FB0_CEILING) = ceiling;
    }
    else
    {
         REG(baseAddr + LCDC_LCDDMA_FB1_BASE) =  base;
         REG(baseAddr + LCDC_LCDDMA_FB1_CEILING) = ceiling;
    }
}


 void LCD_display(void)
{
	RasterEnableClock(SOC_LCDC_0_REGS);

	RasterDisable(SOC_LCDC_0_REGS);                       //关闭raster 功能
#ifdef _LCD_43

 	RasterClkConfig(SOC_LCDC_0_REGS,7833600,192000000);   //配置时钟
#endif

#ifdef _LCD_28

 	RasterClkConfig(SOC_LCDC_0_REGS,5000000,192000000);   //配置时钟
#endif

#ifdef _LCD_7
	 RasterClkConfig(SOC_LCDC_0_REGS,30000000,192000000);   //配置时钟
#endif

        RasterDMAConfig(SOC_LCDC_0_REGS, RASTER_DOUBLE_FRAME_BUFFER,
                    RASTER_BURST_SIZE_16, RASTER_FIFO_THRESHOLD_8,
                    RASTER_BIG_ENDIAN_DISABLE);

        RasterModeConfig(SOC_LCDC_0_REGS, RASTER_DISPLAY_MODE_TFT,
                     RASTER_PALETTE_DATA, RASTER_COLOR, RASTER_RIGHT_ALIGNED);	
	//RasterLSBDataOrderSelect(SOC_LCDC_0_REGS);
	
	 /* disable nibble mode */
       // RasterNibbleModeDisable(SOC_LCDC_0_REGS);

	  /* configuring the polarity of timing parameters of raster controller */
        RasterTiming2Configure(SOC_LCDC_0_REGS, RASTER_FRAME_CLOCK_LOW |
                                            RASTER_LINE_CLOCK_LOW  |
                                            RASTER_PIXEL_CLOCK_HIGH |
                                            RASTER_SYNC_EDGE_RISING|
                                            RASTER_SYNC_CTRL_ACTIVE|
                                            RASTER_AC_BIAS_HIGH     , 0, 255);
#ifdef _LCD_43

	/* configuring horizontal timing parameter */
       RasterHparamConfig(SOC_LCDC_0_REGS, 240, 41, 2, 2);

        /* configuring vertical timing parameters */
       RasterVparamConfig(SOC_LCDC_0_REGS, 320, 10, 3, 3);
 #endif
#ifdef _LCD_28

	/* configuring horizontal timing parameter */
       RasterHparamConfig(SOC_LCDC_0_REGS, 240, 9, 31, 11);

        /* configuring vertical timing parameters */
       RasterVparamConfig(SOC_LCDC_0_REGS, 320, 5, 8, 4);
 #endif
#ifdef  _LCD_7
	RasterHparamConfig(SOC_LCDC_0_REGS, 240, 9, 31, 11);

        /* configuring vertical timing parameters */
       RasterVparamConfig(SOC_LCDC_0_REGS, 320, 5, 8, 4);
#endif
         /* configuring fifo delay to */
        RasterFIFODMADelayConfig(SOC_LCDC_0_REGS, 128);

#ifdef  _LCD_7
	 /* configuring the base ceiling */
       RasterDMAFBConfig(SOC_LCDC_0_REGS,
                      (unsigned int)image1,
                      (unsigned int)image1 + 153630,
                      0);

       RasterDMAFBConfig(SOC_LCDC_0_REGS,
                      (unsigned int)image1,
                      (unsigned int)image1 + 153630,
                      1);
#endif
#ifdef _LCD_43

	RasterDMAFBConfig(SOC_LCDC_0_REGS,
                      (unsigned int)image1,
                      (unsigned int)image1 + 153630,
                      0);

       RasterDMAFBConfig(SOC_LCDC_0_REGS,
                      (unsigned int)image1,
                      (unsigned int)image1 + 153630,
                      1);
#endif
#ifdef _LCD_28

	RasterDMAFBConfig(SOC_LCDC_0_REGS,
                      (unsigned int)image1,
                      (unsigned int)image1 + 153630,
                      0);

       RasterDMAFBConfig(SOC_LCDC_0_REGS,
                      (unsigned int)image1,
                      (unsigned int)image1 + 153630,
                      1);
#endif
        RasterEnable(SOC_LCDC_0_REGS);	


	*(volatile unsigned int *)(0x481AE000 + 0x134) &= ~(1 << 17);
        *(volatile unsigned int *)(0x481AE000 + 0x194) |= (1 << 17);
}

void lcd_init_logo()
{
	//nand_info_t *nand;
	char *s;
	struct mmc *mmc;
	
	mmc_initialize(gd->bd);
	mmc = find_mmc_device(0);

	mmc_init(mmc);
	//nand = &nand_info[nand_curr_device];
	//nand_read_options_t opts;
//	*(volatile unsigned int *)(0x481AE000 + 0x134) &= ~(1 << 17);
  //      *(volatile unsigned int *)(0x481AE000 + 0x194) |= (1 << 17);
	 fat_register_device(&mmc->block_dev,1);
	file_fat_read("logo.bin",(unsigned char *)ptr_lcd,0xBB800);

        unsigned long size = 0xBB800;
       // unsigned int size = 0x3FC00;	
	//nand_read(nand,0x180000,&size,(unsigned char *)(0x82000000));
	ptr_lcd =(unsigned short *)(0x82000000);

	//file_fat_read("logo.bmp",ptr_lcd,0x96000);	
       int count=0;

        for(count=0;count<16;count++)
        {
           *(image1 + count) = array_head[count];
        }

#if 0
        for(count = 16 ;count < 384016;count ++)
        {
         *(image1 + count) = *(ptr_lcd+count-16);
	 //*(image1 + count) = 0x001F;
        }
#endif

#if 1
	for(count = 16 ;count < 53600;count ++)
        {
         *(image1 + count) = 0x001F;
        }
	for(count = 53600 ;count < 153616;count ++)
        {
         *(image1 + count) = 0x00F8;
        }
#endif

/*
       for(count=16;count<128016;count++)
        {
           *(image1 + count) = 0x0000;
        }
        for(count=128016; count < 256016 ;count++)
        {
           *(image1 + count) = *(ptr_lcd + count-128016);
        }
        for(count=256016;count < 384016;count++)
        {
           *(image1 + count) = 0x0000;
        }
*/
//	count = *(volatile unsigned int *)(0x4830E000);
//	printf("%x\r\n",count);


       LCD_display();


}
