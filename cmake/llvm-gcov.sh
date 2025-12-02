#!/bin/bash
# Wrapper script to use llvm-cov as a drop-in replacement for gcov with lcov
exec llvm-cov gcov "$@"
