export type ID = string

//From the server:

export type Poll = {
	id: ID,
	question: string,
	description: string,
	optionA: string,
	optionB: string,
	predictionsA: number,
	predictionsB: number,
	votesA: number,
	votesB: number,
	revealed: Boolean
}

export type UserVote = {
	votedA: Boolean
	predictedA: Boolean
}

export type WorldState = (
	{
		polls: Poll[]
	} & (
		{
			type: "host",
			hostingRoomCode: string
		} | {
			type: "participant",
			votes: {[id: ID]: UserVote?}
		}
	) 
) | {
	type: "error"
	error: "host-vote" | "invalid-poll-vote" | "thread-error" | "participant-poll-creation" | "participant-reveal" | "invalid-poll-reveal" | "participant-poll-delete" | "invalid-poll-delete" | "CLIENT" | "SOCKET"
	retry: boolean
} | {
	type: "connecting"
}

//To the server
export type ServerMessage = (
	{
		type: "join-hello",
		roomCode: string,
		identity: string
	} 
	| 
	{
		type: "host-hello",
		identity: string,
	}
	|
	{
		type: "vote",
		for: ID,
		votedA: boolean
		predictedA: boolean
	}
	|
	{
		type: "create-poll",
		question: string,
		description: string,
		optionA: string,
		optionB: string
	}
	|
	{
		type: "reveal-poll",
		id: ID
	}
	|
	{
		type: "delete-poll",
		id: ID
	}
)