import re
import urlparse
import random
from collections import defaultdict
import operator

def extract_features(INPUT_FILE_1, INPUT_FILE_2, OUTPUT_FILE):
    inp1 = open(INPUT_FILE_1)
    inp2 = open(INPUT_FILE_2)
    out = open(OUTPUT_FILE, 'w')
    
    max_url = 1000
    urls = []

    tmp = []
    for url in inp1:
        tmp += [url]
    inp1.close()
    random.shuffle(tmp)
    
    urls += tmp[:max_url]
    
    tmp = []
    for url in inp2:
        tmp += [url]
    inp2.close()
    random.shuffle(tmp)
    
    urls += tmp[:max_url]

    features = defaultdict(int)
    
    s1 = re.compile(r'^[a-zA-Z]+[0-9]+[a-zA-Z]*$')
    s2 = re.compile(r'^[a-zA-Z]*[0-9]+[a-zA-Z]+$')
    
    for url in urls:
        parse = urlparse.urlparse(url.strip('\n').strip('/'))
        path = parse.path[1:].split('/')

        features['segments:%s' % len(path)] += 1
        for i, segment in enumerate(path):
            ext = segment.rsplit('.', 1)
            
            features['segment_name_%s:%s' % (i, segment)] += 1
                
            if segment.isdigit():
                features['segment_[0-9]_%s:1' % i] += 1
                
            elif len(s1.findall(segment)) + len(s2.findall(segment)) > 0:
                features['segment_substr[0-9]_%s:1' % i] += 1
                
            elif len(ext) == 2:
                if len(s1.findall(ext[0])) + len(s2.findall(ext[0])) > 0:
                    features['segment_ext_substr[0-9]_%s:%s' % (i, ext[1])] += 1
                else:
                    features['segment_ext_%s:%s' % (i, ext[1])] += 1
                    
            features['segment_len_%s:%s' % (i, len(segment))] += 1

        for name, value in urlparse.parse_qs(parse.query).items():
            features['param_name:%s' % name] += 1
            features['param:%s=%s' % (name, value)] += 1
        
    
    for f, v in sorted(features.items(), key=operator.itemgetter(1), reverse=True):
        if v < 100:
            break
        out.write(f + '\t' + str(v) + '\n')