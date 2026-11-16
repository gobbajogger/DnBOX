# SMC pickup expected header repair

The retained fixture compared the entire lower header against status centered at120 with no battery. Current reviewed R22 peek centers status at108 and includes battery219,5. Actual before run first mismatch was219,10 (559d vs4100). The repaired expectation keeps the entire region comparison, uses current text position and battery, and additionally asserts battery outline/cells. All acquisition/write/feedback/ramp tests are unchanged.

Actual repaired run PASS856,048 assertions, including278popup/76peek pickup hint checks. Independent source review accepted this narrowly scoped fixture correction; no production change or full visual qualification. The before log is retained, not relabeled as product failure.
