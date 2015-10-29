// Copyright (c) 2015 Klaralvdalens Datakonsult AB (KDAB).
// All rights reserved. Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#ifndef PHANTOMJS_KEYEVENTS_H
#define PHANTOMJS_KEYEVENTS_H

#if OS_LINUX
#include "keyevents_linux.h"
#elif OS_WINDOWS
#include "keyevents_windows.h"
#endif

#endif
