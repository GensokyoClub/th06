const CACHE_NAME = 'eosd-v1';
const ASSETS = [
    'th06_debug.html',
    'th06_debug.js',
    'th06_debug.wasm',
    'th06_debug.data'
];

self.addEventListener('install', (event) => {
    event.waitUntil(
        caches.open(CACHE_NAME)
        .then((cache) => cache.addAll(ASSETS))
        .then(() => self.skipWaiting())
    );
});

self.addEventListener('activate', (event) => {
    event.waitUntil(
        caches.keys().then((keys) =>
        Promise.all(keys.map((k) => k !== CACHE_NAME && caches.delete(k)))
        )
    );
    self.clients.claim();
});

self.addEventListener('fetch', (event) => {
    event.respondWith(
        caches.match(event.request).then((cached) => {
            if (cached) return cached;
            return fetch(event.request).then((response) => {
                if (
                    response.ok &&
                    (event.request.url.endsWith('.wasm') ||
                    event.request.url.endsWith('.data') ||
                    event.request.url.endsWith('.js'))
                ) {
                    const clone = response.clone();
                    caches.open(CACHE_NAME).then((cache) => cache.put(event.request, clone));
                }
                return response;
            });
        })
    );
});
