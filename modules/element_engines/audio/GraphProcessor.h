/*
    graph-processor.hpp - This file is part of Element
    Copyright (C) 2014  Kushview, LLC.  All rights reserved.

    Adapted from the original JUCE AudioProcessorGraph

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef ELEMENT_GRAPH_PROCESSOR_H
#define ELEMENT_GRAPH_PROCESSOR_H


//==============================================================================
/**
    A type of AudioProcessor which plays back a graph of other AudioProcessors.

    Use one of these objects if you want to wire-up a set of AudioProcessors
    and play back the result.

    Processors can be added to the graph as "nodes" using addNode(), and once
    added, you can connect any of their input or output channels to other
    nodes using addConnection().

    To play back a graph through an audio device, you might want to use an
    AudioProcessorPlayer object.
*/
class JUCE_API  GraphProcessor :  public Processor,
                                  public AsyncUpdater
{
public:
    
    /** Creates an empty graph. */
    GraphProcessor();

    /** Destructor.
        Any processor objects that have been added to the graph will also be deleted.
    */
    ~GraphProcessor();

    /** Represents one of the nodes, or processors, in an AudioProcessorGraph.

        To create a node, call ProcessorGraph::addNode().
    */
    class JUCE_API  Node   : public ReferenceCountedObject
    {
    public:
        
        /** The ID number assigned to this node.
            This is assigned by the graph that owns it, and can't be changed.
        */
        const uint32 nodeId;

        /** The actual processor object that this node represents. */
        Processor* audioProcessor() const noexcept           { return proc; }

        AudioPluginInstance* getAudioPluginInstance() const;
        
        /** The actual processor object dynamic_cast'd to ProcType */
        template<class ProcType>
        inline ProcType* processor() const { return dynamic_cast<ProcType*> (proc.get()); }

        /** A set of user-definable properties that are associated with this node.

            This can be used to attach values to the node for whatever purpose seems
            useful. For example, you might store an x and y position if your application
            is displaying the nodes on-screen.
        */
        NamedValueSet properties;

        /** Returns true if the process is a graph */
        bool isSubgraph() const noexcept;

        /** A convenient typedef for referring to a pointer to a node object. */
        typedef ReferenceCountedObjectPtr <Node> Ptr;

        void setGain (const float g) {
            gain.set (g);
        }
        
        float getGain() const { return gain.get(); }
        float getLastGain() const {return lastGain.get(); }
        void updateGain() { lastGain.set (gain.get()); }
        
        ValueTree getMetadata() const { return metadata; }
        void setMetadata (const ValueTree& meta, bool copy = false)
        {
            metadata = (copy) ? meta.createCopy() : meta;
        }
        
        /* returns the parent graph. If one has not been set, then
           this will return nullptr */
        GraphProcessor* getParentGraph() const;
        
    private:
        friend class GraphProcessor;

        const ScopedPointer<Processor> proc;
        bool isPrepared;

        Node (uint32 nodeId, Processor*) noexcept;

        void setParentGraph (GraphProcessor*);
        void prepare (double sampleRate, int blockSize, GraphProcessor*);
        void unprepare();

        AtomicValue<float> gain, lastGain;
        
        ValueTree metadata;
        GraphProcessor* parent;
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Node)
    };

    //==============================================================================
    /** Represents a connection between two channels of two nodes in an AudioProcessorGraph.

        To create a connection, use AudioProcessorGraph::addConnection().
    */
    struct JUCE_API  Connection :  public Arc
    {
    public:
        //==============================================================================
        Connection (uint32 sourceNode, uint32 sourcePort,
                    uint32 destNode, uint32 destPort) noexcept;

    private:

        //==============================================================================
        JUCE_LEAK_DETECTOR (Connection)

    };

    //==============================================================================
    /** Deletes all nodes and connections from this graph.
        Any processor objects in the graph will be deleted.
    */
    void clear();

    /** Returns the number of nodes in the graph. */
    int getNumNodes() const                                         { return nodes.size(); }

    /** Returns a pointer to one of the nodes in the graph.
        This will return nullptr if the index is out of range.
        @see getNodeForId
    */
    Node* getNode (const int index) const                           { return nodes [index]; }

    /** Searches the graph for a node with the given ID number and returns it.
        If no such node was found, this returns nullptr.
        @see getNode
    */
    Node* getNodeForId (const uint32 nodeId) const;

    /** Adds a node to the graph.

        This creates a new node in the graph, for the specified processor. Once you have
        added a processor to the graph, the graph owns it and will delete it later when
        it is no longer needed.

        The optional nodeId parameter lets you specify an ID to use for the node, but
        if the value is already in use, this new node will overwrite the old one.

        If this succeeds, it returns a pointer to the newly-created node.
    */
    Node* addNode (Processor* newProcessor, uint32 nodeId = 0);

    /** Deletes a node within the graph which has the specified ID.

        This will also delete any connections that are attached to this node.
    */
    bool removeNode (uint32 nodeId);

    //==============================================================================
    /** Returns the number of connections in the graph. */
    int getNumConnections() const                                       { return connections.size(); }

    /** Returns a pointer to one of the connections in the graph. */
    const Connection* getConnection (int index) const                   { return connections [index]; }

    /** Searches for a connection between some specified channels.
        If no such connection is found, this returns nullptr.
    */
    const Connection* getConnectionBetween (uint32 sourceNode,
                                            uint32 sourcePort,
                                            uint32 destNode,
                                            uint32 destPort) const;

    /** Returns true if there is a connection between any of the channels of
        two specified nodes.
    */
    bool isConnected (uint32 sourceNode, uint32 destNode) const;

    /** Returns true if it would be legal to connect the specified points. */
    bool canConnect (uint32 sourceNode, uint32 sourcePort,
                     uint32 destNode, uint32 destPort) const;

    /** Attempts to connect two specified channels of two nodes.

        If this isn't allowed (e.g. because you're trying to connect a midi channel
        to an audio one or other such nonsense), then it'll return false.
    */
    bool addConnection (uint32 sourceNode, uint32 sourcePort,
                        uint32 destNode, uint32 destPort);


    bool connectChannels (PortType type, uint32 sourceNode, int32 sourceChannel,
                          uint32 destNode, int32 destChannel);

    /** Deletes the connection with the specified index. */
    void removeConnection (int index);

    /** Deletes any connection between two specified points.
        Returns true if a connection was actually deleted.
    */
    bool removeConnection (uint32 sourceNode, uint32 sourcePort,
                           uint32 destNode, uint32 destPort);

    /** Removes all connections from the specified node. */
    bool disconnectNode (uint32 nodeId);

    /** Returns true if the given connection's channel numbers map on to valid
        channels at each end.
        Even if a connection is valid when created, its status could change if
        a node changes its channel config.
    */
    bool isConnectionLegal (const Connection* connection) const;

    /** Performs a sanity checks of all the connections.

        This might be useful if some of the processors are doing things like changing
        their channel counts, which could render some connections obsolete.
    */
    bool removeIllegalConnections();

    //==============================================================================
    /** A special number that represents the midi channel of a node.

        This is used as a channel index value if you want to refer to the midi input
        or output instead of an audio channel.
    */
    static const int midiChannelIndex;


    //==============================================================================
    /** A special type of Processor that can live inside an ProcessorGraph
        in order to use the audio that comes into and out of the graph itself.

        If you create an AudioGraphIOProcessor in "input" mode, it will act as a
        node in the graph which delivers the audio that is coming into the parent
        graph. This allows you to stream the data to other nodes and process the
        incoming audio.

        Likewise, one of these in "output" mode can be sent data which it will add to
        the sum of data being sent to the graph's output.

        @see AudioProcessorGraph
    */
    class JUCE_API  AudioGraphIOProcessor     : public Processor
    {
    public:
        /** Specifies the mode in which this processor will operate.
        */
        enum IODeviceType
        {
            audioInputNode = 0, /**< In this mode, the processor has output channels
                                     representing all the audio input channels that are
                                     coming into its parent audio graph. */
            audioOutputNode,    /**< In this mode, the processor has input channels
                                     representing all the audio output channels that are
                                     going out of its parent audio graph. */
            midiInputNode,      /**< In this mode, the processor has a midi output which
                                     delivers the same midi data that is arriving at its
                                     parent graph. */
            midiOutputNode,     /**< In this mode, the processor has a midi input and
                                     any data sent to it will be passed out of the parent
                                     graph. */
            numDeviceTypes
        };

        //==============================================================================
        /** Returns the mode of this processor. */
        IODeviceType getType() const                                { return type; }

        /** Returns the parent graph to which this processor belongs, or nullptr if it
            hasn't yet been added to one. */
        GraphProcessor* getParentGraph() const                 { return graph; }

        /** True if this is an audio or midi input. */
        bool isInput() const;
        /** True if this is an audio or midi output. */
        bool isOutput() const;

        //==============================================================================
        AudioGraphIOProcessor (const IODeviceType type);
        ~AudioGraphIOProcessor();

        void fillInPluginDescription (PluginDescription& d) const;

        const String getName() const;

        void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
        void releaseResources();
        void processBlock (AudioSampleBuffer&, MidiBuffer&);

        const String getInputChannelName (int channelIndex) const;
        const String getOutputChannelName (int channelIndex) const;
        bool isInputChannelStereoPair (int index) const;
        bool isOutputChannelStereoPair (int index) const;
        bool silenceInProducesSilenceOut() const;
        double getTailLengthSeconds() const;
        bool acceptsMidi() const;
        bool producesMidi() const;
        bool hasEditor() const;
        AudioProcessorEditor* createEditor();
        int getNumParameters();
        const String getParameterName (int);
        float getParameter (int);
        const String getParameterText (int);
        void setParameter (int, float);

        int getNumPrograms();
        int getCurrentProgram();
        void setCurrentProgram (int);
        const String getProgramName (int);
        void changeProgramName (int, const String&);

        void getStateInformation (MemoryBlock& destData);
        void setStateInformation (const void* data, int sizeInBytes);

        /** @internal */
        void setParentGraph (GraphProcessor*);

    private:

        const IODeviceType type;
        GraphProcessor* graph;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioGraphIOProcessor)
    };



    //==============================================================================
    // AudioProcessor methods:

    virtual const String getName() const;

    virtual void prepareToPlay (double sampleRate, int estimatedBlockSize);
    virtual void releaseResources();

    void processBlock (AudioSampleBuffer&, MidiBuffer&);
    void reset();

    virtual const String getInputChannelName (int channelIndex) const;
    virtual const String getOutputChannelName (int channelIndex) const;
    virtual bool isInputChannelStereoPair (int index) const;
    virtual bool isOutputChannelStereoPair (int index) const;
    virtual bool silenceInProducesSilenceOut() const;
    virtual double getTailLengthSeconds() const;

    virtual bool acceptsMidi() const;
    virtual bool producesMidi() const;

    virtual bool hasEditor() const                          { return false; }
    virtual AudioProcessorEditor* createEditor()            { return nullptr; }

    virtual int getNumParameters()                          { return 0; }
    virtual const String getParameterName (int)             { return String::empty; }
    virtual float getParameter (int)                        { return 0; }
    virtual const String getParameterText (int)             { return String::empty; }
    virtual void setParameter (int, float)                  { }

    virtual int getNumPrograms()                            { return 0; }
    virtual int getCurrentProgram()                         { return 0; }
    virtual void setCurrentProgram (int)                    { }
    virtual const String getProgramName (int)               { return String::empty; }
    virtual void changeProgramName (int, const String&)     { }

    virtual void getStateInformation (MemoryBlock&);
    virtual void setStateInformation (const void* data, int sizeInBytes);

    virtual void fillInPluginDescription (PluginDescription& d) const;

    inline ValueTree getGraphState() const { return graphState.graph; }

protected:
    
    virtual Node* createNode (uint32 nodeId, Processor* proc) { return new Node (nodeId, proc); }
    virtual void preRenderNodes() { }
    virtual void postRenderNodes() { }

private:
    //==============================================================================

    typedef ArcTable<Connection> LookupTable;

    ReferenceCountedArray <Node> nodes;
    OwnedArray <Connection> connections;

    struct GraphState {
        ValueTree graph;
        ValueTree arcs;
    } graphState;

    uint32 lastNodeId;
    AudioSampleBuffer renderingBuffers;
    OwnedArray <MidiBuffer> midiBuffers;
    Array<void*> renderingOps;

    friend class AudioGraphIOProcessor;
    friend class GraphPort;

    AudioSampleBuffer* currentAudioInputBuffer;
    AudioSampleBuffer currentAudioOutputBuffer;
    MidiBuffer* currentMidiInputBuffer;
    MidiBuffer currentMidiOutputBuffer;

    void handleAsyncUpdate();
    void clearRenderingSequence();
    void buildRenderingSequence();
    bool isAnInputTo (uint32 possibleInputId, uint32 possibleDestinationId, int recursionCheck) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphProcessor)
};

#endif   // ELEMENT_GRAPH_PROCESSOR_H
