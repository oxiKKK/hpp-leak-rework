#include "framework.h"

CSequences g_Sequences;

SequenceList::iterator CSequences::begin()
{
	return m_sequences.begin();
}

SequenceList::iterator CSequences::end()
{
	return m_sequences.end();
}

void CSequences::Update()
{
	if (client_static->netchan.incoming_sequence > m_iLastIncomingSequence)
	{
		m_sequences.push_front(CIncomingSequence{ client_static->netchan.incoming_sequence, client_state->time });
		m_iLastIncomingSequence = client_static->netchan.incoming_sequence;
	}

	if (m_sequences.size() > 2048)
		m_sequences.pop_back();
}

void CSequences::Clear()
{
	if (!m_sequences.empty())
		m_sequences.clear();

	m_iLastIncomingSequence = 0;
}