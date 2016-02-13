import daemon
import os

import get_tweets

context = daemon.DaemonContext(
    working_directory=os.getcwd(),
    stdout=open(os.path.join(os.getcwd(), "logs", "STDOUT"), 'w+'),
    stderr=open(os.path.join(os.getcwd(), "logs", "STDERR"), 'w+'),
)

with context:
    print("Starting program...")
    get_tweets.main()
