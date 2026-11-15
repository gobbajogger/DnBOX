# Targeted controlled-order checks

Install/use the project's existing Node Canvas dependency. Supply the exact separately authorized local design HTML rather than deriving state from a screenshot.

```sh
mkdir -p /tmp/dnbox-ui-ordering
CANDIDATE_PATH=/path/to/authorized-design.html \
EXPECTED_SHA=05cf1e7ce46a2efa630955375192b71215bc8741baa808bbd17f0c1619661d6b \
OUTROOT=/tmp/dnbox-ui-ordering node ordering-checks.cjs
```

The loader verifies input identity before execution. The checks deliberately control frame-load order and the old host-close response, then verify that stale UI continuations cannot cancel a newly minted session. Output is local evidence, not a release or device result.
