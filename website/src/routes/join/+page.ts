import { redirect } from '@sveltejs/kit';
import type { PageLoad } from './$types';

export const load = (async ({url}) => {

    const roomCode = url.searchParams.get('room')

    if (!roomCode) {
        return redirect(307, '/')
    }

    return {
        roomCode: roomCode
    };
}) satisfies PageLoad;