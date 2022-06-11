#include "Sequencer.h"

namespace Sequencer{

	std::shared_ptr<Sequence> sequence = std::make_shared<Sequence>();

	std::shared_ptr<Sequence> getSequence(){ return sequence; }


};
