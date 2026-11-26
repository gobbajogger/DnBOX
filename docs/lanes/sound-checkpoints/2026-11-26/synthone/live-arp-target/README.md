# Actual Elite/Pro compilation of factory arp ingress
Apply ../live-arp/factory-arp-ingress.patch to the isolated H16 donor. Compile
using the separate run script with X0X_GEN_CACHE pointed to writable scratch.
The native loader and app link are intentionally not run; no installed or
package output. Use real full-pack compile settings, no guard exceptions and
confirm the new factory-helper symbols in original_runtime.o for both models.
This is native compile proof only. The actual pack installation, untouched
project policy, real app segment boundaries and strict target replay are open.
