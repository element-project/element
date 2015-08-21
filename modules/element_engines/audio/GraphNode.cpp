GraphNode::GraphNode (const uint32 nodeId_, Processor* const processor_) noexcept
    : nodeId (nodeId_),
      proc (processor_),
      isPrepared (false),
      metadata ("metadata")
{
    parent = nullptr;
    gain.set(1.0f); lastGain.set(1.0f);
    inputGain.set(1.0f); lastInputGain.set(1.0f);
    jassert (proc != nullptr);
}

void GraphNode::setInputGain(const float f) {
    inputGain.set(f);
}

void GraphNode::setGain(const float f) {
    gain.set(f);
}

void GraphNode::getPluginDescription (PluginDescription& desc)
{
    if (AudioPluginInstance* i = getAudioPluginInstance())
        i->fillInPluginDescription (desc);
}

void GraphNode::connectAudioTo (const GraphNode* other)
{
    jassert (getParentGraph());
    jassert (getParentGraph() == other->getParentGraph());

    GraphProcessor& graph (*getParentGraph());
    AudioPluginInstance* const src = getAudioPluginInstance();
    AudioPluginInstance* const dst = other->getAudioPluginInstance();
    DBG("Try connecting: " << src->getName() << " to " << dst->getName());

    const int totalChans = jmin (getNumAudioOutputs(), other->getNumAudioInputs());
    bool failed = false;
    for (int chan = 0; chan < totalChans; ++chan)
    {
        failed |= graph.addConnection (
            this->nodeId, Processor::getPortForAudioChannel(src, chan, false),
            other->nodeId, Processor::getPortForAudioChannel(dst, chan, true)
        );
    }

    if (failed)
    {
        DBG("  failed " << src->getName() << " to " << dst->getName());
    }
}

bool GraphNode::isAudioIONode() const
{
    typedef GraphProcessor::AudioGraphIOProcessor IOP;

    if (IOP* iop = dynamic_cast<IOP*> (proc.get()))
    {
        return iop->getType() == IOP::audioInputNode ||
               iop->getType() == IOP::audioOutputNode;
    }

    return false;
}

bool GraphNode::isMidiIONode() const
{
    typedef GraphProcessor::AudioGraphIOProcessor IOP;
    if (IOP* iop = dynamic_cast<IOP*> (proc.get()))
        return iop->getType() == IOP::midiInputNode || iop->getType() == IOP::midiOutputNode;
    return false;
}

int GraphNode::getNumAudioInputs() const
{
    if (AudioPluginInstance* inst = getAudioPluginInstance())
        return inst->getNumInputChannels();
    return 0;
}

int GraphNode::getNumAudioOutputs() const
{
    if (AudioPluginInstance* inst = getAudioPluginInstance())
        return inst->getNumOutputChannels();
    return 0;
}

void GraphNode::setInputRMS (int chan, float val)
{
    if (chan < inRMS.size()) {
        inRMS.getUnchecked(chan)->set(val);
    }
}

void GraphNode::setOutputRMS (int chan, float val)
{
    if (chan < outRMS.size()) {
        outRMS.getUnchecked(chan)->set(val);
    }
}

bool GraphNode::isSuspended() const
{
    if (AudioPluginInstance* inst = getAudioPluginInstance())
        inst->isSuspended();
    return true;
}

void GraphNode::suspendProcessing (const bool shouldBeSuspended)
{
    if (AudioPluginInstance* inst = getAudioPluginInstance())
        inst->suspendProcessing(shouldBeSuspended);
}

uint32 GraphNode::getMidiInputPort() const
{
    return proc->getNthPort (PortType::Atom, 0, true, false);
}

uint32 GraphNode::getMidiOutputPort() const
{
    return proc->getNthPort (PortType::Atom, 0, false, false);
}

bool GraphNode::isSubgraph() const noexcept
{
    return (dynamic_cast<GraphProcessor*> (proc.get()) != nullptr);
}

void GraphNode::prepare (const double sampleRate, const int blockSize,
                                    GraphProcessor* const graph)
{
    if (! isPrepared)
    {
        AudioPluginInstance* instance = getAudioPluginInstance();
        instance->setPlayConfigDetails (instance->getNumInputChannels(),
                                        instance->getNumOutputChannels(),
                                        sampleRate, blockSize);
        setParentGraph (graph);
        instance->prepareToPlay (sampleRate, blockSize);

        inRMS.clearQuick(true);
        for (int i = 0; i < instance->getNumInputChannels(); ++i)
        {
            AtomicValue<float>* avf = new AtomicValue<float>();
            avf->set(0);
            inRMS.add (avf);
        }

        outRMS.clearQuick(true);
        for (int i = 0; i < instance->getNumOutputChannels(); ++i)
        {
            AtomicValue<float>* avf = new AtomicValue<float>();
            avf->set(0);
            outRMS.add(avf);
        }

        isPrepared = true;
    }
}

void GraphNode::unprepare()
{
    if (isPrepared)
    {
        isPrepared = false;
        inRMS.clear(true);
        outRMS.clear(true);
        proc->releaseResources();
        parent = nullptr;
    }
}

AudioPluginInstance* GraphNode::getAudioPluginInstance() const
{
    if (PluginWrapper* wrapper = dynamic_cast<PluginWrapper*> (proc.get()))
        return wrapper->getWrappedAudioPluginInstance();

    return dynamic_cast<AudioPluginInstance*> (proc.get());
}

GraphProcessor* GraphNode::getParentGraph() const
{
    return parent;
}

void GraphNode::setParentGraph (GraphProcessor* const graph)
{
    parent = graph;
    if (GraphProcessor::AudioGraphIOProcessor* const ioProc
            = dynamic_cast<GraphProcessor::AudioGraphIOProcessor*> (proc.get()))
        ioProc->setParentGraph (graph);
    else if (GraphPort* const ioProc = dynamic_cast <GraphPort*> (proc.get()))
        ioProc->setGraph (graph);
}
