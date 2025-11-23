#define __NV_MODULE_ID _8acb91c5_26_GPUMultiplicationKernel_cu_ba7902aa
#define __NV_CUBIN_HANDLE_STORAGE__ extern
#if !defined(__CUDA_INCLUDE_COMPILER_INTERNAL_HEADERS__)
#define __CUDA_INCLUDE_COMPILER_INTERNAL_HEADERS__
#endif
#include "crt/host_runtime.h"
#include "GPUMultiplicationKernel.fatbin.c"
extern void __device_stub__Z19multiplyTreesKernelPK8FlatNodePK14FlatChildEntryS1_S4_PS_PS2_PiS7_S7_i(const struct FlatNode *, const struct FlatChildEntry *, const struct FlatNode *, const struct FlatChildEntry *, struct FlatNode *, struct FlatChildEntry *, int *, int *, int *, int);
extern void __device_stub__Z18intersectionKernelP12CommonOffsetPKiS2_iiS2_S2_Pi(struct CommonOffset *, const int *, const int *, const int, const int, const int *, const int *, int *);
extern void __device_stub__Z22imprIntersectionKernelP12CommonOffsetPKiS2_iiS2_S2_Pi(struct CommonOffset *, const int *, const int *, const int, const int, const int *, const int *, int *);
extern void __device_stub__Z8multiplyP8FlatNodeS0_P14FlatChildEntryS2_(struct FlatNode *, struct FlatNode *, struct FlatChildEntry *, struct FlatChildEntry *);
static void __nv_cudaEntityRegisterCallback(void **);
static void __sti____cudaRegisterAll(void);
#pragma section(".CRT$XCT",read)
__declspec(allocate(".CRT$XCT"))static void (*__dummy_static_init__sti____cudaRegisterAll[])(void) = {__sti____cudaRegisterAll};
void __device_stub__Z19multiplyTreesKernelPK8FlatNodePK14FlatChildEntryS1_S4_PS_PS2_PiS7_S7_i(
const struct FlatNode *__par0, 
const struct FlatChildEntry *__par1, 
const struct FlatNode *__par2, 
const struct FlatChildEntry *__par3, 
struct FlatNode *__par4, 
struct FlatChildEntry *__par5, 
int *__par6, 
int *__par7, 
int *__par8, 
int __par9)
{
__cudaLaunchPrologue(10);
__cudaSetupArgSimple(__par0, 0Ui64);
__cudaSetupArgSimple(__par1, 8Ui64);
__cudaSetupArgSimple(__par2, 16Ui64);
__cudaSetupArgSimple(__par3, 24Ui64);
__cudaSetupArgSimple(__par4, 32Ui64);
__cudaSetupArgSimple(__par5, 40Ui64);
__cudaSetupArgSimple(__par6, 48Ui64);
__cudaSetupArgSimple(__par7, 56Ui64);
__cudaSetupArgSimple(__par8, 64Ui64);
__cudaSetupArgSimple(__par9, 72Ui64);
__cudaLaunch(((char *)((void ( *)(const struct FlatNode *, const struct FlatChildEntry *, const struct FlatNode *, const struct FlatChildEntry *, struct FlatNode *, struct FlatChildEntry *, int *, int *, int *, int))multiplyTreesKernel)), 0U);
}
void multiplyTreesKernel( const struct ::FlatNode *__cuda_0,const struct ::FlatChildEntry *__cuda_1,const struct ::FlatNode *__cuda_2,const struct ::FlatChildEntry *__cuda_3,struct ::FlatNode *__cuda_4,struct ::FlatChildEntry *__cuda_5,int *__cuda_6,int *__cuda_7,int *__cuda_8,int __cuda_9)
{__device_stub__Z19multiplyTreesKernelPK8FlatNodePK14FlatChildEntryS1_S4_PS_PS2_PiS7_S7_i( __cuda_0,__cuda_1,__cuda_2,__cuda_3,__cuda_4,__cuda_5,__cuda_6,__cuda_7,__cuda_8,__cuda_9);
}
#line 1 "x64/Debug/GPUMultiplicationKernel.cudafe1.stub.c"
void __device_stub__Z18intersectionKernelP12CommonOffsetPKiS2_iiS2_S2_Pi(
struct CommonOffset *__par0, 
const int *__par1, 
const int *__par2, 
const int __par3, 
const int __par4, 
const int *__par5, 
const int *__par6, 
int *__par7)
{
__cudaLaunchPrologue(8);
__cudaSetupArgSimple(__par0, 0Ui64);
__cudaSetupArgSimple(__par1, 8Ui64);
__cudaSetupArgSimple(__par2, 16Ui64);
__cudaSetupArgSimple(__par3, 24Ui64);
__cudaSetupArgSimple(__par4, 28Ui64);
__cudaSetupArgSimple(__par5, 32Ui64);
__cudaSetupArgSimple(__par6, 40Ui64);
__cudaSetupArgSimple(__par7, 48Ui64);
__cudaLaunch(((char *)((void ( *)(struct CommonOffset *, const int *, const int *, const int, const int, const int *, const int *, int *))intersectionKernel)), 0U);
}
void intersectionKernel( struct ::CommonOffset *__cuda_0,const int *__cuda_1,const int *__cuda_2,const int __cuda_3,const int __cuda_4,const int *__cuda_5,const int *__cuda_6,int *__cuda_7)
{__device_stub__Z18intersectionKernelP12CommonOffsetPKiS2_iiS2_S2_Pi( __cuda_0,__cuda_1,__cuda_2,__cuda_3,__cuda_4,__cuda_5,__cuda_6,__cuda_7);
}
#line 1 "x64/Debug/GPUMultiplicationKernel.cudafe1.stub.c"
void __device_stub__Z22imprIntersectionKernelP12CommonOffsetPKiS2_iiS2_S2_Pi(
struct CommonOffset *__par0, 
const int *__par1, 
const int *__par2, 
const int __par3, 
const int __par4, 
const int *__par5, 
const int *__par6, 
int *__par7)
{
__cudaLaunchPrologue(8);
__cudaSetupArgSimple(__par0, 0Ui64);
__cudaSetupArgSimple(__par1, 8Ui64);
__cudaSetupArgSimple(__par2, 16Ui64);
__cudaSetupArgSimple(__par3, 24Ui64);
__cudaSetupArgSimple(__par4, 28Ui64);
__cudaSetupArgSimple(__par5, 32Ui64);
__cudaSetupArgSimple(__par6, 40Ui64);
__cudaSetupArgSimple(__par7, 48Ui64);
__cudaLaunch(((char *)((void ( *)(struct CommonOffset *, const int *, const int *, const int, const int, const int *, const int *, int *))imprIntersectionKernel)), 0U);
}
void imprIntersectionKernel( struct ::CommonOffset *__cuda_0,const int *__cuda_1,const int *__cuda_2,const int __cuda_3,const int __cuda_4,const int *__cuda_5,const int *__cuda_6,int *__cuda_7)
{__device_stub__Z22imprIntersectionKernelP12CommonOffsetPKiS2_iiS2_S2_Pi( __cuda_0,__cuda_1,__cuda_2,__cuda_3,__cuda_4,__cuda_5,__cuda_6,__cuda_7);
}
#line 1 "x64/Debug/GPUMultiplicationKernel.cudafe1.stub.c"
void __device_stub__Z8multiplyP8FlatNodeS0_P14FlatChildEntryS2_(
struct FlatNode *__par0, 
struct FlatNode *__par1, 
struct FlatChildEntry *__par2, 
struct FlatChildEntry *__par3)
{
__cudaLaunchPrologue(4);
__cudaSetupArgSimple(__par0, 0Ui64);
__cudaSetupArgSimple(__par1, 8Ui64);
__cudaSetupArgSimple(__par2, 16Ui64);
__cudaSetupArgSimple(__par3, 24Ui64);
__cudaLaunch(((char *)((void ( *)(struct FlatNode *, struct FlatNode *, struct FlatChildEntry *, struct FlatChildEntry *))multiply)), 0U);
}
void multiply( struct ::FlatNode *__cuda_0,struct ::FlatNode *__cuda_1,struct ::FlatChildEntry *__cuda_2,struct ::FlatChildEntry *__cuda_3)
{__device_stub__Z8multiplyP8FlatNodeS0_P14FlatChildEntryS2_( __cuda_0,__cuda_1,__cuda_2,__cuda_3);
}
#line 1 "x64/Debug/GPUMultiplicationKernel.cudafe1.stub.c"
static void __nv_cudaEntityRegisterCallback(
void **__T13)
{
__nv_dummy_param_ref(__T13);
__nv_save_fatbinhandle_for_managed_rt(__T13);
__cudaRegisterEntry(__T13, ((void ( *)(struct FlatNode *, struct FlatNode *, struct FlatChildEntry *, struct FlatChildEntry *))multiply), _Z8multiplyP8FlatNodeS0_P14FlatChildEntryS2_, (-1));
__cudaRegisterEntry(__T13, ((void ( *)(struct CommonOffset *, const int *, const int *, const int, const int, const int *, const int *, int *))imprIntersectionKernel), _Z22imprIntersectionKernelP12CommonOffsetPKiS2_iiS2_S2_Pi, (-1));
__cudaRegisterEntry(__T13, ((void ( *)(struct CommonOffset *, const int *, const int *, const int, const int, const int *, const int *, int *))intersectionKernel), _Z18intersectionKernelP12CommonOffsetPKiS2_iiS2_S2_Pi, (-1));
__cudaRegisterEntry(__T13, ((void ( *)(const struct FlatNode *, const struct FlatChildEntry *, const struct FlatNode *, const struct FlatChildEntry *, struct FlatNode *, struct FlatChildEntry *, int *, int *, int *, int))multiplyTreesKernel), _Z19multiplyTreesKernelPK8FlatNodePK14FlatChildEntryS1_S4_PS_PS2_PiS7_S7_i, (-1));
}
static void __sti____cudaRegisterAll(void)
{
____cudaRegisterLinkedBinary(__nv_cudaEntityRegisterCallback);
}
