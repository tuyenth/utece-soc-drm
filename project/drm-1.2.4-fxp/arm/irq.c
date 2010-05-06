__irq void p_nIRQ_ISR() {
	    //iFlag = 1;
        printf("software Recv IRQ\n");
}

//#ifdef __cplusplus
//extern "C"
//#else
//extern
//#endif
__irq void p_nFIQ_ISR() {
}
