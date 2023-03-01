import type {Plugin} from 'vite';

export default function vitePluginHeader(headers: { [header: string]: string } = {}): Plugin {
    return {
        name: 'vite-plugin-header',
        apply: 'serve',
        configureServer(server) {
            server.middlewares.use((_, resp, next) => {
                // @ts-ignore
                for (const header in headers) {
                    resp.setHeader(header, headers[header]);
                }
                next();
            });
        }
    };
}