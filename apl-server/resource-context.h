#ifndef _RESOURCE_CONTEXT_H_
#define _RESOURCE_CONTEXT_H_

namespace rcar {
    class ResourceContext {
        public:
            ResourceContext(){}
            ~ResourceContext() {}
            const std::string getMongo() const { return _mongo; } 
            void setMongo(const std::string &address) {  _mongo = address; }
        private:
            std::string _mongo;
    };
}; // namespace 
#endif
