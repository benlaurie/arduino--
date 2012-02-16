#!/usr/bin/env python

import sys
import os
import re
import glob
import json
import subprocess
import tempfile
import datetime
from optparse import OptionParser
from urlparse import urlparse

template = 'sizes/sizes.template'
html = 'sizes/sizes.html'
git_sizes_json = 'sizes/git_sizes.json'
recent_sizes_json = 'sizes/recent_sizes.json'
quiet = False

'''Various functions to maintain the sizes files and generate the HTML.

There are two sizes we keep track of:

recent sizes and git sizes.

The recent sizes keep track of the sizes of binary files between commits.
This file (by default recenet_sizes.json) is updated with each make.

The format is:

{
    "4.6.2": {
        "blink.bin": [
            {
                "index": 1, 
                "mtime": 1329313423, 
                "size": 226
            },
            {
                "index": 2, 
                "mtime": 1329313442, 
                "size": 220
            },            
        ]
    }
}

The git sizes hold the historic data of the current branch and should be updated
by calling this script with the "history" command, e.g.
python sizes/sizes.py history

It is recommended to update the history in the git post-commit hook or
when a branch is created from an older commit.

The format is:

[
    {
        "git": {
            "comment": "First cut.", 
            "short": "1a32447", 
            "hash": "1a324470852211e09f383615617d9fd0f159e385", 
            "author": "Ben Laurie", 
            "date": "2011-12-18 22:20:44 +0000", 
            "email": "ben@links.org"
        }, 
        "4.6.2": {
            "test_enc28j60.bin": 1358
        }
    }, 
    ...
]
'''

def run(*args):
    '''Run a command and return stdout and stderr as list of lines'''
    
    fdout, fnout = tempfile.mkstemp()
    fout = os.fdopen(fdout, "w+")
    fderr, fnerr = tempfile.mkstemp()
    ferr = os.fdopen(fderr, "w+")
    
    out, err = None, None
    try:
        p = subprocess.Popen(args, stderr=ferr, stdout=fout)
        rc = p.wait()
        if rc != 0:
            raise subprocess.CalledProcessError(rc, ' '.join(args))

        fout.seek(0)
        ferr.seek(0)
        out = [line.rstrip() for line in fout]
        err = [line.rstrip() for line in ferr]
    finally:
        fout.close()
        os.unlink(fnout)
        ferr.close()
        os.unlink(fnerr)

    return out, err

def silent(*args):
    '''Run a command silently and returns the exit code'''

    fdout, fnout = tempfile.mkstemp()
    fout = os.fdopen(fdout, "w+")
    fderr, fnerr = tempfile.mkstemp()
    ferr = os.fdopen(fderr, "w+")
    p = subprocess.Popen(args, stderr=ferr, stdout=fout)
    rc = p.wait()
    fout.close()
    os.unlink(fnout)
    ferr.close()
    os.unlink(fnerr)

    return rc

def get_make_cmd(flavour):
    '''Get the name of the make command.
    Flavour is one of "gnu" or "bsd"'''
    
    p_gnu_version = re.compile('GNU Make [0-9.]+')
    cmds = ('make', 'bsdmake', 'gmake')
    for c in cmds:
        try:
            out, _ = run(c, '--version')
            if flavour == 'gnu' and p_gnu_version.search(out[0]) is not None:
                return c
        except subprocess.CalledProcessError:
            # bsd make doesn't understand '--version' and return an error
            # (hopefully everywhere)
            if flavour == 'bsd':
                return c

    raise RuntimeError('Cannot find %s make' % flavour.upper())

def avr_gcc_version():
    '''Return the gcc version of avr-gcc.'''
    
    p_version = re.compile('gcc version ([0-9.]+)')
    _, data = run('avr-gcc', '-v')
    for l in data:
        m = p_version.search(l)
        if m:
            return m.group(1)

def git_branch():
    '''Return the current branch'''
    
    p_branch = re.compile('^\*\s+(.+)')
    data, _ = run('git', 'branch')
    for l in data:
        m = p_branch.search(l)
        if m:
            return m.group(1)

    raise ValueError('Cannot find git branch')

def github_url(url):
    '''Rewrite a github ssh read/write URL into the corresponding public URL'''

    p_ssh = re.compile('\w+@([\w._-]+):(\S+)')
    m = p_ssh.search(url)
    if m:
        return 'https://%s/%s' % (m.group(1), m.group(2))

    m = urlparse(url)
    # Force https URLs
    if m.scheme in ['http', 'https']:
        return 'https://%s%s' % (m.netloc, m.path)

    raise ValueError('Cannot parse remote URL')

def git_remote_url(remote):
    '''Return the URL of <remote> (for example "origin")'''

    p_remote = re.compile(remote + '\s+([\w@.:/_-]+)\s+\(fetch\)')
    data, _ = run('git', 'remote', '-v')
    for l in data:
        m = p_remote.search(l)
        if m:
            return m.group(1)

    raise ValueError('Cannot find git remote "%s"' % remote)
    
p_iso_short = re.compile('^([0-9]{4})-([0-9]{2})-([0-9]{2})')
def iso_short_date(s):
    '''Create a date from the short form of an ISO-date'''
    
    m = p_iso_short.search(s)
    if m:
        return datetime.date(int(m.group(1)), int(m.group(2)), int(m.group(3)))

    raise ValueError('invalid iso date')

def git_log(limit = None):
    '''Return the git log as a list of dictionaries with meta-data.
    The oldest log entry is first in the list.'''
    
    # The header names in the format line determine what goes into the metadata
    pretty = "hash: %H%nshort: %h%nauthor: %an%nemail: %ae%ndate: %ai%n" \
        "comment: %s"
    args = ['git', 'log', '--pretty=%s' % pretty]
    if not limit is None:
        args = args + ['-n', str(limit)]

    data, _ = run(*args)

    p_key = re.compile('([A-Za-z_]+): (.*)')
    log = []
    meta = {}
    for line in data:
        line = line.rstrip()
        if line.startswith('hash:'):
            log.append(meta)
            meta = {}
            
        m = p_key.match(line)
        if m:
            meta[m.group(1)] = m.group(2)

    if meta:
        log.append(meta)
        
    log.reverse()
    
    return log

def json_indented(data, initial_indent = 4, indent = 4):
    '''Indent JSON-data'''
    
    s = json.dumps(data, sort_keys=True, indent=indent)
    lines = s.split('\n')
    space = initial_indent * ' '
    for i in range(1, len(lines)):
        lines[i] = space + lines[i]

    return '\n'.join(lines)

def binfiles():
    return glob.glob('*.bin') + glob.glob('test/*.bin')

def clean():
    for b in binfiles():
        os.unlink(b)

    for o in glob.glob('*.o') + glob.glob('test/*.o'):
        os.unlink(o)

    if os.path.exists('.depend'):
        os.unlink('.depend')

def bname(b):
    return os.path.split(b)[1]

def generate(version, git_sizes, recent_sizes, remote='origin'):
    '''Generate HTML for the sizes'''
    
    remote_url = github_url(git_remote_url(remote))

    prune_git_sizes(git_sizes)

    f = open(template, 'r')
    templ = f.read()
    f.close()
    
    f = open(html, 'w')
    f.write(templ % {
        'git_sizes': json_indented(git_sizes, 4, 4),
        'recent_sizes': json_indented(recent_sizes, 4, 4),
        'compiler_version': version,
        'remote_url': remote_url
        })
    f.close()

def read_git_sizes(fname='sizes/git_sizes.json'):
    '''Return the git (historic) sizes or an empty array.'''
    
    try:
        f = open(fname, 'r')
        git_sizes = json.load(f)
        f.close()
    except IOError:
        git_sizes = []
    
    return git_sizes

def equal_sizes(a, b):
    '''Return true if a and b are equal, but ignore the key "git"
    
    a and b have the form { '4.6.2': { 'file.bin': 17 },
                            '4.5.2': { 'file.bin': 23 },
                            'git': {...}'''
    
    if a.keys() != b.keys():
        return False

    for v, s in a.iteritems():
        if v != 'git':
            if s != b[v]:
                return False

    return True

def prune_git_sizes(sizes):
    prune = []
    for idx, info in enumerate(sizes):
        if idx > 0 and equal_sizes(sizes[idx-1], info):
            prune.append(idx)

    for p in reversed(prune):
        del sizes[p]

    return sizes

def read_recent_sizes():
    '''Return the recent sizes on the file system or an empty dict'''
    
    try:
        f = open(recent_sizes_json, 'r')
        recent = json.load(f)
        f.close()
    except IOError:
        recent = {}

    return recent
        
def update_recent(version):
    '''Update the recent sizes.'''

    recent = read_recent_sizes()
    counter = recent.get('counter', 0) + 1

    increment = False
    for b in binfiles():
        size = os.path.getsize(b)
        if not recent.has_key(version):
            recent[version] = {}

        blist = recent[version]
        bn = bname(b)
        if not blist.has_key(bn):
            blist[bn] = []

        bfile = blist[bn]
        # Only add files with changed sizes
        if not len(bfile) or bfile[-1].get('size', None) != size:
            mt = os.path.getmtime(b)
            increment = True
            bfile.append({'index': counter, 'size': size,
                           'mtime': int(mt)})

    if increment:
        counter = counter + 1
    recent['counter'] = counter
    
    return recent

def append_git_size(version, sizes):
    '''Append the current sizes from the working copy in git_sizes format.'''

    hashes = set()
    for s in sizes:
        hashes.add(s['git']['hash'])

    git_info = git_log(1)[0]
    info = {}

    for b in binfiles():
        info[bname(b)] = os.path.getsize(b)

    if not git_info['hash'] in hashes:
        sizes.append({version: info, 'git': git_info})

    return sizes

def write_sizes(sizes, fname):
    f = open(fname, 'w')
    json.dump(sizes, f, indent=4)
    f.close()

def update_history(version):
    '''Update the history in git_sizes.json'''

    branch = git_branch()
    make = get_make_cmd('bsd')
    sizes = read_git_sizes()
    
    revlist, _ = run('git', 'rev-list', branch, '--')

    hashes = set()
    stale = []
    for i, s in enumerate(sizes):
        h = s['git']['hash']
        # populate known hashes
        hashes.add(h)
        if not h in revlist:
            # remove stale entries, i.e. unrelated commits from other branches
            # or commits that were deleted through an interactive rebase
            # (or whatever else git throws our way) 
            print '%s removed (stale)' % h
            stale.append(i)

    for u in reversed(stale):
        del sizes[u]

    try:
        for r in reversed(revlist):
            if not r in hashes:
                rc = subprocess.call(['git', 'checkout', '-q', r])
                if rc:
                    raise RuntimeError('git checkout -q ' + r + ' failed')

                # in case make clean doesn't work
                clean()
                rc = silent(make, '-k', 'clean', 'all')
                append_git_size(version, sizes)
                if not rc:
                    print '%s ok' % r
                else:
                    print '%s make failed' % r
            else:
                if not quiet:
                    print '%s already recorded' % r
    finally:
        subprocess.call(['git', 'checkout', '-q', branch])
        if os.path.exists('.depend'):
            os.unlink('.depend')
        silent(make, '-k', 'clean', 'all')

    sizes.sort(key=lambda x: x['git']['date'])
    return sizes

if __name__ == '__main__':
    parser = OptionParser('usage: %prog OPTIONS recent|generate|history+')
    parser.add_option("-r", "--remote", default='origin',
                      help="the git REMOTE name (default is origin)")
    parser.add_option("-q", "--quiet", action='store_true',
                      help="be somewhat quiet")
    options, args = parser.parse_args()

    quiet = options.quiet

    version = avr_gcc_version()
    recent_sizes = read_recent_sizes()
    git_sizes = read_git_sizes()
    for a in args:
        if a == 'recent':
            recent_sizes = update_recent(version)
            write_sizes(recent_sizes, 'sizes/recent_sizes.json')
        elif a == 'history':
            git_sizes = update_history(version)
            write_sizes(git_sizes, 'sizes/git_sizes.json')
        elif a == 'generate':
            generate(version, git_sizes, recent_sizes, options.remote)
    else:
        generate(version, git_sizes, recent_sizes, options.remote)
        
