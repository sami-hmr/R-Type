alias dev-config='cmake --preset=dev'
alias dev-build='cmake --build build --preset=dev-build -j'

alias tests-config='cmake --preset=tests'
alias tests-build='cmake --build build --preset=tests-build -j'

alias docs-config='cmake --preset=docs'
alias docs-build='cmake --build build/docs --preset=docs-build -j'
alias docs-run='cmake --build build/docs --target docs-run'

alias tests-config='cmake --preset=tests'
alias tests-build='cmake --build build --preset=tests-build -j'
alias tests-run='ctest --preset=dev-test'

alias coverage='cmake --build build/tests --target coverage && xdg-open build/coverage_html/index.html'

alias rel-config='cmake --preset=release'
alias rel-build='cmake --build build/release --preset=release-build -j'

alias format-check='cmake --build build --target format-check'
alias format-fix='cmake --build build --target format-fix'
