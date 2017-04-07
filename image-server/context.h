#ifndef _IMAGE_CONTEXT_H_
#define _IMAGE_CONTEXT_H_

namespace rcar {
    class ImageContext {
        public:
            ImageContext(){}
            ~ImageContext() {}
            const std::string getMongo() const { return _mongo; } 
            void setMongo(const std::string &address) {  _mongo = address; }
        private:
            std::string _mongo;
    };
}; // namespace 
#endif
