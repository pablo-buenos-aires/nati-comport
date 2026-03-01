// Compatibility shim for certain C++Builder XE console/linker configurations
// where the runtime references __InitVCL even for non-VCL console projects.
// This project intentionally does not use VCL.

extern "C" void __InitVCL()
{
}
