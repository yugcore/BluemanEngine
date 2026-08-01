#ifndef DX12_HOST_H
#define DX12_HOST_H

// Global GPU synchronization function (defined in main.cpp)
// Used by ViewportRenderer and other subsystems that need to
// wait for the GPU to finish before releasing resources.
void WaitForGPU();

#endif // DX12_HOST_H
