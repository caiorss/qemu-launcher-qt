import argparse

parent_parser = argparse.ArgumentParser(add_help=False)
parent_parser.add_argument('--user', '-u',
                           default=100, help='username')

parent_parser.add_argument('--debug', default=False, required=False,
                           action='store_true', dest="debug", help='debug flag')


main_parser = argparse.ArgumentParser()

service_subparsers = main_parser.add_subparsers(title="service",
                    dest="service_command")

action_subparser = main_parser.add_subparsers(title="action",
                    dest="action_command")

action_parser = action_subparser.add_parser("second", help="second",
                    parents=[parent_parser])

args = main_parser.parse_args()

print(args)
# print( args["--foo"] )
