# ==========================================================================
#
#   Copyright NumFOCUS
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#          https://www.apache.org/licenses/LICENSE-2.0.txt
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
# ==========================================================================

#
#  Example on the use of the MedianImageFilter
#

import itk

# Test that docstring in snake_case function is replaced by
# docstring from corresponding object.

# Not the default docstring.
assert "Procedural interface for" not in itk.median_image_filter.__doc__
# But the actual filter docstring.
assert "Applies a median filter to an image" in itk.median_image_filter.__doc__
