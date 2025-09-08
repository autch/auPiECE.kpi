class AuPIECEKPI;
void  piece_Context_Init(void);
void  piece_Context_Deinit(void);
void* piece_Context_Create(AuPIECEKPI *decoder);
void  piece_Context_Delete(void *pv);
void  piece_Context_Lock(void *pv);
void  piece_Context_Unlock(void *pv);
