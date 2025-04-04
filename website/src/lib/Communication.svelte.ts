import type { ServerMessage, WorldState } from "./types";

const ADDRESS = "ws://localhost:8080"

export class Communication {
	#access
	#onChange
	#socket: WebSocket | null = null
	#identity: string | null = null

	#retries = 0

	get identity() {
		if (this.#identity) {
			return this.#identity
		} else {
			const savedIdentity = localStorage.getItem("identity")
			if (savedIdentity) {
				this.#identity = savedIdentity
				return savedIdentity
			} else {
				const newIdentity = crypto.randomUUID()
				localStorage.setItem("identity", newIdentity)
				this.#identity = newIdentity
				return newIdentity
			}
		}
	}

	constructor(access: {type: "host"} | {type: "participant", roomCode: string}, onChange: (state: WorldState) => void) {
		this.#access = access
		this.#onChange = onChange
	}

	#processWorldState(state: WorldState) {
		if (state.type === "error") {
			if (state.retry) {
				this.#socket?.close()
				if (this.#retries++ < 5) {
					this.#retries++
					this.begin()
				} else {
					this.#onChange({type: "error", error: "SOCKET", retry: false})
				}
			}
		}
		this.#onChange(state)
	}

	begin() {
		this.#socket = new WebSocket(ADDRESS)
		this.#socket.onopen = () => {
			if (this.#access.type === "host") {
				this.send({type: "host-hello", identity: this.identity})
			} else {
				this.send({type: "join-hello", roomCode: this.#access.roomCode, identity: this.identity})
			}
		}
		this.#socket.onclose = () => {
			this.#processWorldState({type: "connecting"})
			if (this.#retries++ < 5) {
				this.#retries++
				this.begin()
			} else {
				this.#onChange({type: "error", error: "SOCKET", retry: false})
			}
		}
		this.#socket.onerror = () => {
			this.#processWorldState({type: "error", error: "SOCKET", retry: true})
		}
		this.#socket.onmessage = (event) => {
			try {
				this.#processWorldState(JSON.parse(event.data))
			} catch (e) {
				console.error(e)
				this.#processWorldState({type: "error", error: "CLIENT", retry: false})
			}
		}
	}

	send(message: ServerMessage) {
		this.#socket?.send(JSON.stringify(message))
	}

	end() {
		this.#socket?.close()
	}
}