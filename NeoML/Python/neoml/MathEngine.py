""" Copyright (c) 2017-2020 ABBYY Production LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
--------------------------------------------------------------------------------------------------------------*/
"""

import neoml.PythonWrapper as PythonWrapper


class MathEngine:
    """
    """
    def __init__(self, internal):
        if not isinstance(internal, PythonWrapper.MathEngine):
            raise ValueError('The `internal` must be PythonWrapper.MathEngine')

        self._internal = internal

    @property
    def peak_memory_usage(self):
        """The peak memory usage achieved during processing
        """
        return self._internal.get_peak_memory_usage()

    def clean_up(self):
        """Releases all temporary resources allocated for the current thread.
        """
        return self._internal.clean_up()

# ----------------------------------------------------------------------------------------------------------------------


class CpuMathEngine(MathEngine):
    """
    """
    def __init__(self, thread_count=None):
        if thread_count is None:
            thread_count = 0

        if isinstance(thread_count, PythonWrapper.MathEngine):
            super().__init__(thread_count)
            return

        internal = PythonWrapper.MathEngine('cpu', int(thread_count), 0)
        super().__init__(internal)

# ----------------------------------------------------------------------------------------------------------------------


class GpuMathEngine(MathEngine):
    """
    """
    def __init__(self, gpu_index):
        if gpu_index is None:
            gpu_index = 0

        if isinstance(gpu_index, PythonWrapper.MathEngine):
            super().__init__(gpu_index)
            return

        if gpu_index < 0 or gpu_index >= len(enum_gpu()):
            raise ValueError("GPU with index `gpu_index` doesn't exist.")

        internal = PythonWrapper.MathEngine('gpu', 0, int(gpu_index))
        super().__init__(internal)

    @property
    def info(self):
        """Gets the device information
        """
        return self._internal.get_info()

# ----------------------------------------------------------------------------------------------------------------------


def enum_gpu():
    """Allows you to access the information about all available GPUs
    """
    return PythonWrapper.enum_gpu()


def default_math_engine():
    """Returns default CPU math engine
    """
    return MathEngine(PythonWrapper.default_math_engine())