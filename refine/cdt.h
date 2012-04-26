#ifndef __P2TC_REFINE_CDT_H__
#define __P2TC_REFINE_CDT_H__

typedef struct
{
  P2trMesh *mesh;
  P2trPSLG *outline;
} P2trCDT;

P2trCDT* p2tr_cdt_new (P2tCDT *cdt);

#endif