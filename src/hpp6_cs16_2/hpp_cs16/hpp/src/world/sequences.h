class CIncomingSequence
{
public:
	int seq;
	double time;
};

using SequenceList = std::deque<CIncomingSequence>;

class CSequences
{
public:
	SequenceList::iterator begin();
	SequenceList::iterator end();

	void Update();
	void Clear();

private:
	int m_iLastIncomingSequence;
	SequenceList m_sequences;
};

extern CSequences g_Sequences;