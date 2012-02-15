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
    '''Run a command silently and return the exit code'''
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
    # flavour is one of 'gnu' or 'bsd'
    p_gnu_version = re.compile('GNU Make [0-9.]+')
    cmds = ('make', 'bsdmake', 'gmake')
    for c in cmds:
        try:
            out, _ = run(c, '--version')
            if flavour == 'gnu' and p_gnu_version.search(out[0]) is not None:
                return c
        except subprocess.CalledProcessError:
            # bsd make doesn't understand '--version' and throws an error
            if flavour == 'bsd':
                return c

    raise RuntimeError('Cannot find %s make' % flavour.upper())

def avr_gcc_version():
    p_version = re.compile('gcc version ([0-9.]+)')
    _, data = run('avr-gcc', '-v')
    for l in data:
        m = p_version.search(l)
        if m:
            return m.group(1)

def github_url(url):
    # This is github-specific
    p_ssh = re.compile('\w+@([\w._-]+):(\S+)')
    m = p_ssh.search(url)
    if m:
        return 'https://%s/%s' % (m.group(1), m.group(2))

    m = urlparse(url)
    # Force https URLs
    if m.scheme in ['http', 'https']:
        return 'https://%s%s' % (m.netloc, m.path)

    raise ValueError('Cannot parse remote URL')

def git_branch():
    p_branch = re.compile('^\*\s+(.+)')
    data, _ = run('git', 'branch')
    for l in data:
        m = p_branch.search(l)
        if m:
            return m.group(1)

    raise ValueError('Cannot find git branch')

def git_remote_url(remote):
    p_remote = re.compile(remote + '\s+([\w@.:/_-]+)\s+\(fetch\)')
    data, _ = run('git', 'remote', '-v')
    for l in data:
        m = p_remote.search(l)
        if m:
            return m.group(1)

    raise ValueError('Cannot find git remote "%s"' % remote)
    
p_iso_short = re.compile('^([0-9]{4})-([0-9]{2})-([0-9]{2})')
def iso_short_date(s):
    m = p_iso_short.search(s)
    if m:
        return datetime.date(int(m.group(1)), int(m.group(2)), int(m.group(3)))

    raise ValueError('invalid iso date')

def git_log(limit = None):    
    # The header names in the format line determine what goes into the metadata,
    # the full body (%B) is needed for the sizes
    pretty = "hash: %H%nshort: %h%nauthor: %an%nemail: %ae%ndate: %ai%n" \
        "comment: %s%n%B"
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
    s = json.dumps(data, sort_keys=True, indent=indent)
    lines = s.split('\n')
    space = initial_indent * ' '
    for i in range(1, len(lines)):
        lines[i] = space + lines[i]

    return '\n'.join(lines)

def binfiles():
    return glob.glob('test/*.bin')

def bname(b):
    return os.path.split(b)[1]

def generate(version, git_sizes, recent_sizes):
    remote_url = github_url(git_remote_url(options.remote))

    prune_git_sizes(git_sizes)

    f = open(options.template, 'r')
    template = f.read()
    f.close()
    
    f = open(options.output, 'w')
    f.write(template % {
        'git_sizes': json_indented(git_sizes, 4, 4),
        'recent_sizes': json_indented(recent_sizes, 4, 4),
        'compiler_version': version,
        'remote_url': remote_url
        })
    f.close()

def git_sizes():
    try:
        f = open(options.git_sizes, 'r')
        git_sizes = json.load(f)
        f.close()
    except IOError:
        git_sizes = []
    
    return git_sizes

def equal_sizes(a, b):
    #a has the form { '4.6.2': { 'file.bin': 17 },
    #                 '4.5.2': { 'file.bin': 23 },
    #                 'git': {...}
    
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

def update(version):
    try:
        f = open(options.sizes, 'r')
        recent_sizes = json.load(f)
        f.close()
    except IOError:
        recent_sizes = {}

    counter = recent_sizes.get('counter', 0) + 1

    increment = False
    for b in binfiles():
        size = os.path.getsize(b)
        if not recent_sizes.has_key(version):
            recent_sizes[version] = {}

        blist = recent_sizes[version]
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
    recent_sizes['counter'] = counter
    
    f = open(options.sizes, 'w')
    json.dump(recent_sizes, f, sort_keys=True, indent=4)
    f.close()
    return recent_sizes

def append_git_size(sizes):

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

def write_git_sizes(sizes):
    sizes.sort(key=lambda x: x['git']['date'])
    f = open(options.git_sizes, 'w')
    json.dump(sizes, f, indent=4)
    f.close()

def history(branch):
    make = get_make_cmd('bsd')
    sizes = git_sizes()
    revlist, _ = run('git', 'rev-list', branch, '--')
    hashes = set()
    stale = []
    for i, s in enumerate(sizes):
        h = s['git']['hash']
        hashes.add(h)
        if not h in revlist:
            print '%s removed (stale)' % h
            stale.append(i)

    # remove stale entries, i.e. unrelated commits from other branches or
    # commits that were deleted through an interactive rebase (or whatever else
    # git throws our way) 
    for u in reversed(stale):
        del sizes[u]

    try:
        for r in revlist:
            if not r in hashes:
                rc = subprocess.call(['git', 'checkout', '-q', r])
                if rc:
                    raise RuntimeError('git checkout -q ' + r + ' failed')

                if os.path.exists('.depend'):
                    os.unlink('.depend')
                rc = silent(make, 'clean', 'all')
                append_git_size(sizes)
                if not rc:
                    print '%s ok' % r
                else:
                    print '%s make failed' % r
            else:
                if not options.quiet:
                    print '%s already recorded' % r
    finally:
        subprocess.call(['git', 'checkout', '-q', branch])
        if os.path.exists('.depend'):
            os.unlink('.depend')
        silent(make, 'clean', 'all')

    return sizes

if __name__ == '__main__':
    parser = OptionParser('usage: %prog OPTIONS')
    parser.add_option("-s", "--sizes", default='sizes/recent_sizes.json',
                      help="json file (input and output)")
    parser.add_option("-g", "--git-sizes", default='sizes/git_sizes.json',
                      help="json file (input and output)")
    parser.add_option("-o", "--output", default='sizes/sizes.html',
                      help="write HTML output to FILE")
    parser.add_option("-t", "--template", default='sizes/sizes.template',
                      help="read HTML from TEMPLATE")
    parser.add_option("-r", "--remote", default='origin',
                      help="the git REMOTE name (default is origin)")
    parser.add_option("-q", "--quiet", action='store_true',
                      help="be more quiet")
    options, args = parser.parse_args()

    version = avr_gcc_version()
    for a in args:
        if a == 'generate':
            generate(version, git_sizes(), update(version))
        elif a == 'append':
            sizes = git_sizes()
            append_git_size(sizes)
            write_git_sizes(sizes)
        elif a == 'history':
            branch = git_branch()
            sizes = history(branch)
            write_git_sizes(sizes)
            generate(version, sizes, update(version))
    else:
        generate(version, git_sizes(), update(version))
        
