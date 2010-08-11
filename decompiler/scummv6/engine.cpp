/* ScummVM Tools
 * Copyright (C) 2010 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "engine.h"
#include "disassembler.h"
#include "codegen.h"

::Disassembler *Scumm::v6::Scummv6Engine::getDisassembler(std::vector<Instruction> &insts) {
	return new Scummv6Disassembler(insts);
}

uint32 Scumm::v6::Scummv6Engine::getDestAddress(ConstInstIterator it) const {
	switch(it->_type) {
	case kJump:
	case kCondJump:
		return it->_params[0].getUnsigned();
	case kJumpRel:
	case kCondJumpRel:
		return it->_params[0].getSigned() + it->_address + 3;
	default:
		return 0;
	}
}

::CodeGenerator *Scumm::v6::Scummv6Engine::getCodeGenerator(std::ostream &output) {
	return new Scummv6CodeGenerator(this, output);
}
