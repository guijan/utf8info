# Copyright (c) 2022 Guilherme Janczak <guilherme.janczak@yandex.com>
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

project('utf8info', 'c', version: '0.1', license: 'ISC',
        default_options: ['c_std=c99', 'b_lto=true', 'warning_level=3'])

cc = meson.get_compiler('c')

# Makes all functions, including BSD functions, visible on GNU.
add_project_arguments(['-D_GNU_SOURCE'], language: 'c')

funcs = {
  'err': '#include <err.h>',
  'errx': '#include <err.h>',
  'strcasestr': '#include <string.h>',
  'strlcat': '#include <string.h>',
  'strlcpy': '#include <string.h>',
}
libbsd_dep = dependency('', required: false)
foreach func, header: funcs
  if not cc.has_function(func, prefix: header)
    libbsd_dep = dependency('libbsd-overlay')
    break
  endif
endforeach

executable('utf8info',
           'utf8info.c',
           install: true,
           dependencies: libbsd_dep,
           win_subsystem: 'console')
