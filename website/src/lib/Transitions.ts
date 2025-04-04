import { cubicInOut } from "svelte/easing"

export function expandVertically(node: HTMLElement, params: {duration: number}, options: { direction: 'in' | 'out' | 'both' }) {
	const correctHeight = node.clientHeight
	return {
		duration: params.duration,
		css: (t:number) => {
			return `height: ${t*correctHeight}px; overflow:hidden`
		},
		easing: cubicInOut
	}
}