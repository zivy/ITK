#!/usr/bin/env bash
#=============================================================================
# Copyright 2023 NumFOCUS
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#=============================================================================
# This script adapted from the KWSys utilities

usage='usage: cmake-format.bash [<options>] [--]

    --help                     Print usage plus more detailed help.

    --cmake-format <tool>      Use given cmake-format tool.

    --amend                    Filter files changed by HEAD.
    --cached                   Filter files locally staged for commit.
    --modified                 Filter files locally modified from HEAD.
    --last                     Filter files locally modified from HEAD~1
    --tracked                  Filter files tracked by Git.
'

help="$usage"'
Example to format locally modified files:

    Utilities/Maintenance/cmake-format.bash --modified

Example to format locally modified files staged for commit:

    Utilities/Maintenance/cmake-format.bash --cached

Example to format files modified by the most recent commit:

    Utilities/Maintenance/cmake-format.bash --amend

Example to format all files:

    Utilities/Maintenance/cmake-format.bash --tracked

Example to format the current topic:

    git filter-branch \
      --tree-filter "Utilities/Maintenance/cmake-format.bash --tracked" \
      main..
'

die() {
    echo "$@" 1>&2; exit 1
}

#-----------------------------------------------------------------------------

# Parse command-line arguments.
cmake_format=''
mode=''
while test "$#" != 0; do
    case "$1" in
    --amend) mode="amend" ;;
    --cached) mode="cached" ;;
    --cmake-format) shift; cmake_format="$1" ;;
    --help) echo "$help"; exit 0 ;;
    --modified) mode="modified" ;;
    --last) mode="last" ;;
    --tracked) mode="tracked" ;;
    --) shift ; break ;;
    -*) die "$usage" ;;
    *) break ;;
    esac
    shift
done
test "$#" = 0 || die "$usage"

# Find a default tool.
tools='
  cmake-format
'
if test "x$cmake_format" = "x"; then
    for tool in $tools; do
        if type -p "$tool" >/dev/null; then
            cmake_format="$tool"
            break
        fi
    done
fi

# Verify that we have a tool.
if ! type -p "$cmake_format" >/dev/null; then
    echo "Unable to locate a 'cmake-format' tool."
    exit 1
fi

# Select listing mode.
case "$mode" in
    '')       echo "$usage"; exit 0 ;;
    amend)    git_ls='git diff-tree  --diff-filter=AM --name-only HEAD -r --no-commit-id' ;;
    cached)   git_ls='git diff-index --diff-filter=AM --name-only HEAD --cached' ;;
    modified) git_ls='git diff-index --diff-filter=AM --name-only HEAD' ;;
    last)     git_ls='git diff-index --diff-filter=AM --name-only HEAD~1' ;;
    tracked)  git_ls='git ls-files' ;;
    *) die "invalid mode: $mode" ;;
esac

# List files as selected above.
$git_ls |

  # Select sources with our attribute.
  git check-attr --stdin hooks.style         |
  grep -e 'hooks.style: .*cmakeformat'       |
  grep -v 'ThirdParty'                       |
  grep -v 'ExternalData.cmake'               |
  grep -v 'Modules/Remote'                   |
  sed -n 's/:[^:]*:[^:]*$//p'                |
  # Update sources in-place.
  tr '\n' '\0'                               |
  xargs -0 "$cmake_format" -c ./Utilities/CMakeFormat/config.yaml -i
