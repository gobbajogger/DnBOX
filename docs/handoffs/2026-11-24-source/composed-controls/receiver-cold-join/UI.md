# Combined UI memory follow-up

Apply `undo-span-fit.patch`, then `title-fit.patch`, after the three source patches. Undo shares the original bounds checks, failure text, edit-hold path, owner mask updates and five saved ranges. The title graft shares unchanged labels and page-title bounds and reduces pool usage. On the combined cold-data and receiver donor, the paired net saving was 332/336 B XIP and 396 B POOL. See FIT.md for the rejected final maps. No feature or audio tests, runtime boot, device write, installation, or release took place.
